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
