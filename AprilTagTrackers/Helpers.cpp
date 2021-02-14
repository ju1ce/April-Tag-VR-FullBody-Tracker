#include "Helpers.h"

void drawMarker(cv::Mat frame, std::vector<cv::Point2f> corners, cv::Scalar color)
{
    cv::line(frame, cv::Point2i(int(corners[0].x), int(corners[0].y)), cv::Point2i(int(corners[1].x), int(corners[1].y)), color, 2);
    cv::line(frame, cv::Point2i(int(corners[1].x), int(corners[1].y)), cv::Point2i(int(corners[2].x), int(corners[2].y)), color, 2);
    cv::line(frame, cv::Point2i(int(corners[2].x), int(corners[2].y)), cv::Point2i(int(corners[3].x), int(corners[3].y)), color, 2);
    cv::line(frame, cv::Point2i(int(corners[3].x), int(corners[3].y)), cv::Point2i(int(corners[0].x), int(corners[0].y)), color, 2);
}
void transformMarkerSpace(std::vector<cv::Point3f> modelMarker, cv::Vec3d boardRvec, cv::Vec3d boardTvec, cv::Vec3d rvec, cv::Vec3d tvec, std::vector<cv::Point3f>* out)
{
	//if any button was pressed, we try to add visible markers to our board

			//convert the board rotation vector to rotation matrix
	cv::Mat rmat;
	//for timing our detection
	//start = clock();

	cv::Rodrigues(boardRvec, rmat);


	//with rotation matrix and translation vector we create the translation matrix
	cv::Mat mtranslation = cv::Mat_<double>(4, 4);
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			mtranslation.at<double>(x, y) = rmat.at<double>(x, y);
		}
	}
	for (int x = 0; x < 3; x++)
	{
		mtranslation.at<double>(x, 3) = boardTvec[x];
		mtranslation.at<double>(3, x) = 0;
	}
	mtranslation.at<double>(3, 3) = 1;


	//othervise, we create our markers translation matrix
	cv::Mat rmatin;
	Rodrigues(rvec, rmatin);

	cv::Mat mtranslationin = cv::Mat_<double>(4, 4);
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			mtranslationin.at<double>(x, y) = rmatin.at<double>(x, y);
		}
	}
	for (int x = 0; x < 3; x++)
	{
		mtranslationin.at<double>(x, 3) = tvec[x];
		mtranslationin.at<double>(3, x) = 0;
	}
	mtranslationin.at<double>(3, 3) = 1;

	cv::Mat rpos = cv::Mat_<double>(4, 1);

	//we transform our model marker from our marker space to board space
	out->clear();
	for (int y = 0; y < modelMarker.size(); y++)
	{
		//transform corner of model marker from vector to mat for calculation
		rpos.at<double>(0, 0) = modelMarker[y].x;
		rpos.at<double>(1, 0) = modelMarker[y].y;
		rpos.at<double>(2, 0) = modelMarker[y].z;
		rpos.at<double>(3, 0) = 1;

		//multipy model marker corner with markers translation matrix to get its position in camera(global) space
		rpos = mtranslationin * rpos;
		//multiply marker corner in camera space with the inverse of the translation matrix of our board to put it into local space of our board
		rpos = mtranslation.inv() * rpos;

		//cout << ids[i] << " :: marker\n" << rpos << "\n";

		out->push_back(cv::Point3f(rpos.at<double>(0, 0), rpos.at<double>(1, 0), rpos.at<double>(2, 0)));
	}

	//we add our marker corners to our board
}

void getMedianMarker(std::vector<std::vector<cv::Point3f>> markerList, std::vector<cv::Point3f>* median)
{
	median->clear();
	for (int j = 0; j < markerList[0].size(); j++)
	{
		std::vector<float> pointsx;
		std::vector<float> pointsy;
		std::vector<float> pointsz;
		
		for (int i = 0; i < markerList.size(); i++)
		{
			pointsx.push_back(markerList[i][j].x);
			pointsy.push_back(markerList[i][j].y);
			pointsz.push_back(markerList[i][j].z);
		}
		std::sort(pointsx.begin(), pointsx.end());
		std::sort(pointsy.begin(), pointsy.end());
		std::sort(pointsz.begin(), pointsz.end());

		median->push_back(cv::Point3f(pointsx[pointsx.size() / 2], pointsy[pointsy.size() / 2], pointsz[pointsz.size() / 2]));
	}
}

Quaternion<double> rodr2quat(double x, double y, double z)
{

	double theta;
	theta = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
	x = x / theta;
	y = y / theta;
	z = z / theta;

	double qx = x * sin(theta / 2);
	double qy = y * sin(theta / 2);
	double qz = z * sin(theta / 2);
	double qw = cos(theta / 2);

	Quaternion<double> q(qw, qx, qy, qz);

	return q;
}
cv::Mat getSpaceCalib(cv::Vec3d rvec, cv::Vec3d tvec, double xOffset, double yOffset, double zOffset)
{
	cv::Mat rmat;
	cv::Rodrigues(rvec, rmat);
	cv::Mat wtranslation = cv::Mat_<double>(4, 4);
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			wtranslation.at<double>(x, y) = rmat.at<double>(x, y);
		}
	}
	for (int x = 0; x < 3; x++)
	{
		wtranslation.at<double>(x, 3) = (tvec)[x];
		wtranslation.at<double>(3, x) = 0;
	}
	wtranslation.at<double>(3, 3) = 1;
	wtranslation = wtranslation.inv();
	//we add the values we sent to our driver to our matrix
	wtranslation.at<double>(0, 3) += xOffset;
	wtranslation.at<double>(1, 3) += yOffset;
	wtranslation.at<double>(2, 3) += zOffset;

	return wtranslation;
}

cv::Mat getSpaceCalibEuler(cv::Vec3d rvec, cv::Vec3d tvec, double xOffset, double yOffset, double zOffset)
{
	cv::Mat rmat;
	//cv::Rodrigues(rvec, rmat);
	cv::Vec3f eulers = rvec;
	rmat = eulerAnglesToRotationMatrix(eulers);
	cv::Mat wtranslation = cv::Mat_<double>(4, 4);
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			wtranslation.at<double>(x, y) = rmat.at<double>(x, y);
		}
	}
	for (int x = 0; x < 3; x++)
	{
		wtranslation.at<double>(x, 3) = (tvec)[x];
		wtranslation.at<double>(3, x) = 0;
	}
	wtranslation.at<double>(3, 3) = 1;
	//wtranslation = wtranslation.inv();
	//we add the values we sent to our driver to our matrix
	wtranslation.at<double>(0, 3) += xOffset;
	wtranslation.at<double>(1, 3) += yOffset;
	wtranslation.at<double>(2, 3) += zOffset;

	return wtranslation;
}

// Calculates rotation matrix given euler angles.
cv::Mat eulerAnglesToRotationMatrix(cv::Vec3f& theta)
{
	// Calculate rotation about x axis
	cv::Mat R_x = (cv::Mat_<double>(3, 3) <<
		1, 0, 0,
		0, cos(theta[0]), -sin(theta[0]),
		0, sin(theta[0]), cos(theta[0])
		);

	// Calculate rotation about y axis
	cv::Mat R_y = (cv::Mat_<double>(3, 3) <<
		cos(theta[1]), 0, sin(theta[1]),
		0, 1, 0,
		-sin(theta[1]), 0, cos(theta[1])
		);

	// Calculate rotation about z axis
	cv::Mat R_z = (cv::Mat_<double>(3, 3) <<
		cos(theta[2]), -sin(theta[2]), 0,
		sin(theta[2]), cos(theta[2]), 0,
		0, 0, 1);


	// Combined rotation matrix
	cv::Mat R = R_y * R_x * R_z;

	return R;

}

// Checks if a matrix is a valid rotation matrix.
bool isRotationMatrix(cv::Mat& R)
{
	cv::Mat Rt;
	transpose(R, Rt);
	cv::Mat shouldBeIdentity = Rt * R;
	cv::Mat I = cv::Mat::eye(3, 3, shouldBeIdentity.type());

	return  norm(I, shouldBeIdentity) < 1e-6;

}

// Calculates rotation matrix to euler angles
// The result is the same as MATLAB except the order
// of the euler angles ( x and z are swapped ).
cv::Vec3f rotationMatrixToEulerAngles(cv::Mat& R)
{

	assert(isRotationMatrix(R));

	float sy = sqrt(R.at<double>(0, 0) * R.at<double>(0, 0) + R.at<double>(1, 0) * R.at<double>(1, 0));

	bool singular = sy < 1e-6; // If

	float x, y, z;
	if (!singular)
	{
		x = atan2(R.at<double>(2, 1), R.at<double>(2, 2));
		y = atan2(-R.at<double>(2, 0), sy);
		z = atan2(R.at<double>(1, 0), R.at<double>(0, 0));
	}
	else
	{
		x = atan2(-R.at<double>(1, 2), R.at<double>(1, 1));
		y = atan2(-R.at<double>(2, 0), sy);
		z = 0;
	}
	return cv::Vec3f(x, y, z);

}

inline double SIGN(double x) {
	return (x >= 0.0f) ? +1.0f : -1.0f;
}

inline double NORM(double a, double b, double c, double d) {
	return sqrt(a * a + b * b + c * c + d * d);
}

Quaternion<double> mRot2Quat(const cv::Mat& m) {
	double r11 = m.at<double>(0, 0);
	double r12 = m.at<double>(0, 1);
	double r13 = m.at<double>(0, 2);
	double r21 = m.at<double>(1, 0);
	double r22 = m.at<double>(1, 1);
	double r23 = m.at<double>(1, 2);
	double r31 = m.at<double>(2, 0);
	double r32 = m.at<double>(2, 1);
	double r33 = m.at<double>(2, 2);
	double q0 = (r11 + r22 + r33 + 1.0f) / 4.0f;
	double q1 = (r11 - r22 - r33 + 1.0f) / 4.0f;
	double q2 = (-r11 + r22 - r33 + 1.0f) / 4.0f;
	double q3 = (-r11 - r22 + r33 + 1.0f) / 4.0f;
	if (q0 < 0.0f) {
		q0 = 0.0f;
	}
	if (q1 < 0.0f) {
		q1 = 0.0f;
	}
	if (q2 < 0.0f) {
		q2 = 0.0f;
	}
	if (q3 < 0.0f) {
		q3 = 0.0f;
	}
	q0 = sqrt(q0);
	q1 = sqrt(q1);
	q2 = sqrt(q2);
	q3 = sqrt(q3);
	if (q0 >= q1 && q0 >= q2 && q0 >= q3) {
		q0 *= +1.0f;
		q1 *= SIGN(r32 - r23);
		q2 *= SIGN(r13 - r31);
		q3 *= SIGN(r21 - r12);
	}
	else if (q1 >= q0 && q1 >= q2 && q1 >= q3) {
		q0 *= SIGN(r32 - r23);
		q1 *= +1.0f;
		q2 *= SIGN(r21 + r12);
		q3 *= SIGN(r13 + r31);
	}
	else if (q2 >= q0 && q2 >= q1 && q2 >= q3) {
		q0 *= SIGN(r13 - r31);
		q1 *= SIGN(r21 + r12);
		q2 *= +1.0f;
		q3 *= SIGN(r32 + r23);
	}
	else if (q3 >= q0 && q3 >= q1 && q3 >= q2) {
		q0 *= SIGN(r21 - r12);
		q1 *= SIGN(r31 + r13);
		q2 *= SIGN(r32 + r23);
		q3 *= +1.0f;
	}
	else {
		printf("coding error\n");
	}
	double r = NORM(q0, q1, q2, q3);
	q0 /= r;
	q1 /= r;
	q2 /= r;
	q3 /= r;

	Quaternion<double> q;
	q.x = q1;
	q.y = q2;
	q.z = q3;
	q.w = q0;

	return q;
}
void ShowMessage(std::string text, bool stopOnOk)
{
	std::thread th{ [=]() {
		wxMessageDialog* dial = new wxMessageDialog(NULL,
		text, wxT("Message"), wxOK);
	dial->ShowModal();
	} };
	
	th.detach();
}