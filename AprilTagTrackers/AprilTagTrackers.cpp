#include <iostream>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include "apriltag.h"
#include "tagStandard41h12.h" //65cm
#include "tag36h11.h"
#include "tagCircle21h7.h"

using namespace std;
using namespace cv;

Mat retImage;
bool imgReady = false;

void getImages(VideoCapture inputVideo)
{
	//an infinite loop that will be reading images from our stream and writing them to our variable
	while (true)
	{
		inputVideo >> retImage;
		//set that new image is ready
		imgReady = true;
		//imshow("out", retImage);
	}
}

int main()
{
	cv::VideoCapture inputVideo;
	//inputVideo.open("http://192.168.0.11:8080/video");
	inputVideo.open(1);

	TermCriteria termcrit(TermCriteria::COUNT | TermCriteria::EPS, 3, 0.001);
	Size winSize(17, 17);

	inputVideo.set(cv::CAP_PROP_FPS, 60);
	inputVideo.set(cv::CAP_PROP_EXPOSURE, 10);

	apriltag_detector_t* td = apriltag_detector_create();
	td->quad_decimate = 1;
	apriltag_family_t* tf = tagStandard41h12_create();
	apriltag_detector_add_family(td, tf);

	cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << 5.3437183745774701e+02, 0., 3.0507796498876326e+02, 0., 5.3479864731102782e+02, 2.3361808596384694e+02, 0., 0., 1.);
	cv::Mat distCoeffs = (cv::Mat_<double>(1, 5) << -1.0999772162221377e-01, 1.5540578784566195e-01, -4.2558866507839374e-04, 5.7480288581676197e-04, -3.9691669990206194e-02);

	int i = 0;

	Mat prevFrame, frame, gray;;
	std::vector<int> prevIds;
	std::vector<std::vector<cv::Point2f> > prevCorners;
	std::vector<cv::Point2f> prevCenters;

	inputVideo >> frame;
	cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

	image_u8_t* im = image_u8_create_stride(gray.cols, gray.rows, gray.cols);

	thread imageThread(getImages, inputVideo);

	while (true)
	{
		if (!imgReady)
			continue;
		retImage.copyTo(frame);
		imgReady = false;

		clock_t start, end;
		//for timing our detection
		start = clock();

		cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
		/*
		if (prevCenters.size() > 0)
		{
			cout << "heloooo" << endl;
			//Then define your mask image
			cv::Mat mask = cv::Mat::zeros(gray.size(), gray.type());

			cv::Mat dstImage = cv::Mat::zeros(gray.size(), gray.type());

			//I assume you want to draw the circle at the center of your image, with a radius of 50
			for (int i = 0; i < prevCenters.size(); i++)
			{
				cv::circle(mask, prevCenters[i], 150, cv::Scalar(255, 0, 0), -1, 8, 0);
			}

			//Now you can copy your source image to destination image with masking
			gray.copyTo(dstImage, mask);

			gray = dstImage;
		}
		*/
		im->buf = gray.data;

		zarray_t* detections = apriltag_detector_detect(td, im);

		std::cout << zarray_size(detections) << std::endl;

		end = clock();
		//time of marker detection
		std::cout << double(end - start) / double(CLOCKS_PER_SEC) << "\n";

		std::vector<int> ids;
		std::vector<std::vector<cv::Point2f> > corners;
		std::vector<cv::Point2f> centers;

		for (int i = 0; i < zarray_size(detections); i++) {
			apriltag_detection_t* det;
			zarray_get(detections, i, &det);

			ids.push_back(det->id);
			centers.push_back(Point2f(det->c[0], det->c[1]));

			std::vector<cv::Point2f> temp;

			for (int j = 3; j >= 0; j--)
			{
				temp.push_back(cv::Point2f(det->p[j][0], det->p[j][1]));
			}

			corners.push_back(temp);
			/*
			line(frame, cv::Point(det->p[0][0], det->p[0][1]),
				cv::Point(det->p[1][0], det->p[1][1]),
				cv::Scalar(0, 0xff, 0), 1);
			line(frame, cv::Point(det->p[0][0], det->p[0][1]),
				cv::Point(det->p[3][0], det->p[3][1]),
				cv::Scalar(0, 0, 0xff), 1);
			line(frame, cv::Point(det->p[1][0], det->p[1][1]),
				cv::Point(det->p[2][0], det->p[2][1]),
				cv::Scalar(0xff, 0, 0), 1);
			line(frame, cv::Point(det->p[2][0], det->p[2][1]),
				cv::Point(det->p[3][0], det->p[3][1]),
				cv::Scalar(0xff, 0, 0), 1);
				*/
		}
		apriltag_detections_destroy(detections);

		prevCenters = centers;

		/*			MARKER TRACKING
		for (int i = 0; i < prevIds.size(); i++)
		{
			bool found = false;
			for (int j = 0; j < ids.size(); j++)
			{
				if (prevIds[i] == ids[j])
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				vector<uchar> status;
				vector<float> err;
				vector<Point2f> tempCorners;
				vector<Point2f> testCorners;

				calcOpticalFlowPyrLK(prevFrame, frame, prevCorners[i], tempCorners, status, err, winSize, 3, termcrit);
				cornerSubPix(gray, tempCorners, Size(3, 3), Size(-1, -1), termcrit);
				calcOpticalFlowPyrLK(frame, prevFrame, tempCorners, testCorners, status, err, winSize, 3, termcrit);
				cornerSubPix(gray, testCorners, Size(3, 3), Size(-1, -1), termcrit);

				bool valid = true;
				for (int j = 0; j < prevCorners[i].size(); j++)
				{
					cout << pow((testCorners[j].x - prevCorners[i][j].x), 2) + pow((testCorners[j].y - prevCorners[i][j].y), 2) << "::";
					if (status[j] != 1 || err[j] > 30 || pow((testCorners[j].x-prevCorners[i][j].x),2) + pow((testCorners[j].y - prevCorners[i][j].y),2) > 10)
						valid = false;
				}

				cout << endl;

				if (valid)
				{
					ids.push_back(prevIds[i]);

					corners.push_back(tempCorners);
				}
			}
		}

		end = clock();
		//time of marker detection
		std::cout << double(end - start) / double(CLOCKS_PER_SEC) << "\n";
		*/
		prevIds = ids;
		prevCorners = corners;
		prevFrame = frame.clone();

		std::vector<cv::Vec3d> rvecs, tvecs;

		if (ids.size() > 0)
		{
			cv::aruco::drawDetectedMarkers(frame, corners, ids);
			cv::aruco::estimatePoseSingleMarkers(corners, 1, cameraMatrix, distCoeffs, rvecs, tvecs);
			for (int i = 0; i < rvecs.size(); ++i) {
				//draw axis for each marker
				auto rvec = rvecs[i];	//rotation vector of our marker
				auto tvec = tvecs[i];	//translation vector of our marker

				//rotation/translation vectors are shown as offset of our camera from the marker

				cv::aruco::drawAxis(frame, cameraMatrix, distCoeffs, rvec, tvec, 10);
			}
		}

		end = clock();
		//time of marker detection
		std::cout << double(end - start) / double(CLOCKS_PER_SEC) << "\n";

		cv::imshow("lol", frame);
		cv::waitKey(1);

		end = clock();
		//time of marker detection
		std::cout << double(end - start) / double(CLOCKS_PER_SEC) << "\n";
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
