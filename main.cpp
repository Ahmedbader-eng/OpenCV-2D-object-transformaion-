#include <opencv2/opencv.hpp>

#define PI           3.14159265358979323846		// define pi value

int main(int argc, char** argv)
{
	// initialising matrix objects to load the image and create threshold images
	cv::Mat image;
	cv::Mat black_A_threshold, blue_A_threshold;

	image = cv::imread("Reference Image.png", CV_LOAD_IMAGE_UNCHANGED);	// reading the image from the project file

																		// if the image is not found the program will terminate
	if (image.empty()) {
		std::cout << "image not found" << std::endl;
		return -1;
	}

	// create a window to display the image
	// the window name is image with the flag CV_WINDOW_AUTOSIZE to display the image with its original size
	// this is limited to the screan resolution so the window may scale the image to fit
	cv::namedWindow("image", CV_WINDOW_AUTOSIZE);
	cv::imshow("image", image); // show the image on the window named image

								// detect blue colour which is the blue A and put the out but in blue_A_threshold Mat object
	cv::inRange(image, cv::Scalar(255, 0, 0), cv::Scalar(255, 0, 0), blue_A_threshold);

	// detect black colour which is the black A and put the out but in blacl_A_threshold Mat object
	cv::inRange(image, cv::Scalar(0, 0, 0), cv::Scalar(0, 0, 0), black_A_threshold);

	/// Contours are detected in the thresholded image.
	// a vector of vector of points to store the blue_contours or black_contours i.e. the blue/blaack colour detected
	std::vector<std::vector<cv::Point> > blue_contours;
	cv::findContours(blue_A_threshold, blue_contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	std::vector<std::vector<cv::Point> > black_contours;
	cv::findContours(black_A_threshold, black_contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	// a deep copy of the original image to draw on it
	cv::Mat drawing = image.clone();

	// varibles to store the points of the source and destination images
	cv::Point2f src_points[4];
	cv::Point2f dst_points[4];

	// RoyatedResct class gives the rotation angle in degrees the center and the four points of the rectangle
	cv::RotatedRect blue_A;
	cv::RotatedRect black_A;

	for (int i = 0; i < blue_contours.size(); i++) {
		// to overcome the noise detect only blue object with area larger than 1000 pixels^2
		if (cv::contourArea(blue_contours[i]) > 1000) {

			// draw counters which is the blue colour detected
			// drawing is a copy of the input image to draw on it
			// i is the index to indicat which contour to draw
			// contours can be drawn in the desired colour here they are drawn in red
			//the thickness of the line that draws the contour is set to 3 pixels
			cv::drawContours(drawing, blue_contours, i, cv::Scalar(0, 0, 255), 3);

			// initialising an object named box in RotatedRect class
			// minAreaRect function is used to find the minimum area bounding rectangle possible rotated 
			/* the minAreaRect function returns a 2D box structure which contains following detals - ( center (x,y),
			(width, height), angle of rotation )*/
			// the returned structure can be used to draw the rectangle
			cv::RotatedRect box = cv::minAreaRect(blue_contours[i]);

			// coppy the retuned structure to a varible outside the loop
			blue_A = box;

			// store the points of the rectangle in a Point2F variable to draw a rectangle 
			cv::Point2f vertices[4];

			// copy the points to the variable vertices 
			box.points(vertices);

			// loop 4 times to draw the rectangle by drawing lines between the 4 points of the rectangle
			for (int j = 0; j < 4; j++) {
				cv::line(drawing, vertices[j], vertices[(j + 1) % 4], cv::Scalar(0, 255, 0), 3);

				//assign each point the src_points varible outside the loop
				src_points[j] = vertices[j];
			}

		}
	}

	// Repeat the drawing and finding the points processes but for the black A
	for (int i = 0; i < black_contours.size(); i++) {
		if (cv::contourArea(black_contours[i]) > 1000) {
			cv::drawContours(drawing, black_contours, i, cv::Scalar(0, 0, 255), 3);
			cv::RotatedRect box = cv::minAreaRect(black_contours[i]);

			black_A = box;

			cv::Point2f vertices[4];
			box.points(vertices);

			for (int j = 0; j < 4; j++) {
				cv::line(drawing, vertices[j], vertices[(j + 1) % 4], cv::Scalar(0, 0, 255), 3);

				dst_points[j] = vertices[j];
			}
		}
	}

	// calculate the translation between the blue A and the black A
	// te translation can be calculated from the RotatedRect objects of both blue and black As
	float deltaX = black_A.center.x - blue_A.center.x;
	float deltaY = black_A.center.y - blue_A.center.y;

	// store the translation in a vector named translation
	std::vector<float> translation = { deltaX, deltaY, 0.0 };

	// printing the translation vector by accessing each element
	std::cout << "translation vector:" << std::endl;
	std::cout << "[" << translation.at(0) << std::endl;
	std::cout << " " << translation.at(1) << std::endl;
	std::cout << " " << translation.at(2) << " ]" << std::endl;
	std::cout << std::endl;

	// storing the blue and black A corners points to print them
	cv::Point2f blue_vertices[4];
	blue_A.points(blue_vertices);

	cv::Point2f black_vertices[4];
	black_A.points(black_vertices);

	// printing the centers of blue and black As
	std::cout << "blue A center: " << blue_A.center << "	black A center: " << black_A.center << std::endl;
	std::cout << std::endl;

	// printing the corners of blue and black As
	std::cout << "blue A verices : " << blue_vertices[0] << blue_vertices[1] << blue_vertices[2] << blue_vertices[3] << std::endl;
	std::cout << "black A verices : " << black_vertices[0] << black_vertices[1] << black_vertices[2] << black_vertices[3] << std::endl;
	std::cout << std::endl;

	// creating a window for the blue and black threshold images 
	cv::namedWindow("Blue detection", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("Black detection", CV_WINDOW_AUTOSIZE);

	// showing the threshold images of blue and black As
	cv::imshow("Blue detection", blue_A_threshold);
	cv::imshow("Black detection", black_A_threshold);

	// creating a batrix object to store the prespective transform
	// the prespective transform is found using getPerspectiveTransform function
	/*this function calculates the 3x3 prespective transform matrix using the corners of
	the source location and the destination location*/
	// source corners of the blue A and destination corners of the black A
	cv::Mat prespTransformMatrix;
	prespTransformMatrix = cv::getPerspectiveTransform(src_points, dst_points);

	// printing the Prespective Transform Matrix
	std::cout << "Prespective Transform Matrix:" << std::endl;
	std::cout << prespTransformMatrix << std::endl;
	std::cout << std::endl;

	// initialize a varible to store the matched ponints
	// the matched points are the the corners of a subtracted image between a moving image and the black A threshold image
	// the moving image is the blue A threshold image rotated and translated using the Prespective Transform Matrix
	cv::Point2f matchedPoints[4];

	// temp is a varible of type intger to search for the minimum area 
	// we dont know the minimum size so we set this varible biggest possible araa to search for the lowest area
	int temp = black_A_threshold.size().width * black_A_threshold.size().height;


	// loop to find the minimum area
	/*the rotation angle is set between 120-200 degrees because in this range
	the moving A will match with the black A threshold */
	// this range will ensure the moving A is in the frame
	for (int i = 120; i < 180; i++) {
		// rotating the A to find the matching points
		prespTransformMatrix.at<double>(0, 0) = -cos((PI / 180)*i);
		prespTransformMatrix.at<double>(0, 1) = sin((PI / 180)*i);
		prespTransformMatrix.at<double>(1, 0) = sin((PI / 180)*i);
		prespTransformMatrix.at<double>(1, 1) = cos((PI / 180)*i);

		// initialis a Mat object named prespTransformedImage to apply the prespective transform to the blue A image
		// warpPerspective function will transform the blue A using the prespective transform matrix
		/* warpPerspective takes input image, output image of the same size and type as the input image,
		prespective transform matrix and output image size*/
		cv::Mat prespTransformedImage;
		prespTransformedImage.create(blue_A_threshold.size(), blue_A_threshold.type());
		cv::warpPerspective(blue_A_threshold, prespTransformedImage, prespTransformMatrix, prespTransformedImage.size());

		// initalise a Mat object named subtracted_image to find the matching points
		// subtracting the prespective transformed image from the black A threshold image
		// if the subtraction result is 0 we then know this is the matching image and we can then find its key points
		cv::Mat subtracted_image;
		cv::subtract(prespTransformedImage, black_A_threshold, subtracted_image);

		/* detecting the A in the subtracted image the same way as detecting the blue and black A except that
		the subtracted image is already threshold image*/
		std::vector<std::vector<cv::Point> > subtracted_image_contours;
		cv::findContours(subtracted_image, subtracted_image_contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

		for (int i = 0; i < subtracted_image_contours.size(); i++) {

			// check if there is area detected			
			if (cv::contourArea(subtracted_image_contours[i]) > 0) {

				// find the minimum area detected
				if (cv::contourArea(subtracted_image_contours[i]) < temp) {

					cv::RotatedRect box = cv::minAreaRect(subtracted_image_contours[i]);

					temp = cv::contourArea(subtracted_image_contours[i]);

					cv::Point2f vertices[4];
					box.points(vertices);
					for (int j = 0; j < 4; j++) {
						// copy the points of the minimum area image to matchedPoint varible
						matchedPoints[j] = vertices[j];

					}
				}
			}
		}


		// creating a window to show the drawn rectangles
		cv::namedWindow("drawing", CV_WINDOW_AUTOSIZE);
		cv::imshow("drawing", drawing);

		// creating a window to show the Presp Transformed Image
		cv::namedWindow("Presp Transformed Image", CV_WINDOW_AUTOSIZE);
		cv::imshow("Presp Transformed Image", prespTransformedImage);

		// create a window to show the subtracted images
		cv::namedWindow("subtracted_image", CV_WINDOW_AUTOSIZE);
		cv::imshow("subtracted_image", subtracted_image);


		// wait 1 millisecond for user input
		char c = cv::waitKey(1);
		if (c == 'q') {	// if q is pressed break from the loop
			break;
		}
		else if (c == 'p') { // if p is pressed pause/play
			cv::waitKey(0);
			continue;
		}
	}

	// initalise two vectors of 2 points for findHomography function to find the homogenous transform
	// one for the source image corners
	// the other for the matching points corners at the destination
	std::vector<cv::Point2f> sorce_points;
	std::vector<cv::Point2f> destination_points;

	// loop to store the corners of the source image and the matching points in their vectors
	for (int i = 0; i < 4; i++) {
		sorce_points.push_back(src_points[i]);
		destination_points.push_back(matchedPoints[i]);
	}

	// initialise a Mat object called homogeneous_transform to store the homogenous transformation matrix
	/* the function finds the prespective transform between two planes using the coordinates of
	the points in the original plane and target plane*/
	// findHomography returns a 3x3 homogenous matrix
	cv::Mat homogeneous_transform = cv::findHomography(sorce_points, destination_points);

	// printing the 3x3 homogenous matrix
	std::cout << "findhomography output:" << std::endl;
	std::cout << homogeneous_transform << std::endl;
	std::cout << std::endl;

	// printing 4x4 homogenous transform matrix
	std::cout << "homogeneous transform: [ " << homogeneous_transform.at<double>(0, 0) << "	" << homogeneous_transform.at<double>(0, 1) << "	" << 0 << "	" << homogeneous_transform.at<double>(0, 2) << std::endl;
	std::cout << "			" << homogeneous_transform.at<double>(1, 0) << "	" << homogeneous_transform.at<double>(1, 1) << "	" << 0 << "	" << homogeneous_transform.at<double>(1, 2) << std::endl;
	std::cout << "			" << homogeneous_transform.at<double>(2, 0) << "	" << homogeneous_transform.at<double>(2, 1) << "	" << 1 << "	" << 0 << std::endl;
	std::cout << "			" << 0 << "		" << 0 << "		" << 0 << "		" << homogeneous_transform.at<double>(2, 2) << " ]" << std::endl;

	// wait for any key to be pressed 
	cv::waitKey(0);
	// destroy all open windows and terminate
	cv::destroyAllWindows();

	return 0;
}