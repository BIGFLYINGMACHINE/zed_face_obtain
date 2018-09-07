
 // ZED includes
#include <sl_zed/Camera.hpp>

// OpenCV includes
#include <opencv2/opencv.hpp>
#include <iostream>
// Sample includes
#include <SaveDepth.hpp>
#include <string>
using namespace sl;

cv::Mat slMat2cvMat(Mat& input);
void printHelp();

int main() {

	Camera zed;

	cv::namedWindow("left");
	cv::namedWindow("right");
	//要定义windows才能使用setMousecallBack
	
	InitParameters init_params;
	init_params.camera_resolution = RESOLUTION_HD720;//分辨率
	init_params.camera_fps = 30;//帧数
	
	ERROR_CODE err = zed.open(init_params);
	if (err != SUCCESS)
	{
		printf("%s\n", toString(err).c_str());
		zed.close();
		return 1;
	}
    // Prepare new image size to retrieve half-resolution images
    Resolution image_size = zed.getResolution();
    int new_width = image_size.width;
    int new_height = image_size.height;

    // To share data between sl::Mat and cv::Mat, use slMat2cvMat()
    // Only the headers and pointer to the sl::Mat are copied, not the data itself
    Mat image_zed_left(new_width, new_height, MAT_TYPE_8U_C4);
    cv::Mat cv_left = slMat2cvMat(image_zed_left);
	Mat image_zed_right(new_width, new_height, MAT_TYPE_8U_C4);
	cv::Mat cv_right = slMat2cvMat(image_zed_right);
	
	char name[20];
	static int count=0;
	
    // Loop until 'q' is pressed
    char key = ' ';
    while (key != 'q') {

        if (zed.grab() == SUCCESS) 
		{

            // Retrieve the left image, depth image in half-resolution
            zed.retrieveImage(image_zed_left, VIEW_LEFT, MEM_CPU, new_width, new_height);
			zed.retrieveImage(image_zed_right, VIEW_RIGHT, MEM_CPU, new_width, new_height);
			cv_left = slMat2cvMat(image_zed_left);
			cv_right = slMat2cvMat(image_zed_right);
			cv::cvtColor(cv_left, cv_left, cv::COLOR_BGRA2BGR);
			cv::cvtColor(cv_right, cv_right, cv::COLOR_BGRA2BGR);
			cv_left.convertTo(cv_left, CV_8U);
			cv_right.convertTo(cv_right, CV_8U);
			assert(cv_left.channels() == 3&&cv_right.channels()==3);
			for (int i = 0; i < 2; i++)
			{
				cv::Mat src_img, gray_img;
				if (i == 0)
				{
					std::cout << "cv_left.channels:" << cv_left.channels() << std::endl;
					src_img = cv_left;
				}
				if (i == 1)
				{
					src_img = cv_right;
				}
				std::vector<cv::Rect> faces;
				std::cout << "channels:"<<src_img.channels() << std::endl;
				cv::cvtColor(src_img, gray_img, CV_RGB2GRAY);
				cv::CascadeClassifier cascade;
				cascade.load("D:\\opencv-3.4.1\\build\\install\\etc\\haarcascades\\haarcascade_frontalface_default.xml");
				cascade.detectMultiScale(gray_img, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
				for (int i = 0; i < faces.size(); i++)
				{
					cv::Mat face;
					src_img(faces[i]).copyTo(face);
					sprintf(name, "./myfaces/face%d.jpg", count++);
					cv::imwrite(name, face);
				}
			}
            // Handle key event
			cv::imshow("left", cv_left);
			cv::imshow("right", cv_right);
            key = cv::waitKey(10);
            processKeyEvent(zed, key);
        }
		else
		{
			printf("can not grab");
		}
    }
    zed.close();
    return 0;
}

/**
* Conversion function between sl::Mat and cv::Mat
**/
cv::Mat slMat2cvMat(Mat& input) {
    // Mapping between MAT_TYPE and CV_TYPE
    int cv_type = -1;
    switch (input.getDataType()) {
        case MAT_TYPE_32F_C1: cv_type = CV_32FC1; break;
        case MAT_TYPE_32F_C2: cv_type = CV_32FC2; break;
        case MAT_TYPE_32F_C3: cv_type = CV_32FC3; break;
        case MAT_TYPE_32F_C4: cv_type = CV_32FC4; break;
        case MAT_TYPE_8U_C1: cv_type = CV_8UC1; break;
        case MAT_TYPE_8U_C2: cv_type = CV_8UC2; break;
        case MAT_TYPE_8U_C3: cv_type = CV_8UC3; break;
        case MAT_TYPE_8U_C4: cv_type = CV_8UC4; break;
        default: break;
    }

    // Since cv::Mat data requires a uchar* pointer, we get the uchar1 pointer from sl::Mat (getPtr<T>())
    // cv::Mat and sl::Mat will share a single memory structure
    return cv::Mat(input.getHeight(), input.getWidth(), cv_type, input.getPtr<sl::uchar1>(MEM_CPU));
}

/**
* This function displays help in console
**/
void printHelp() {
    std::cout << " Press 's' to save Side by side images" << std::endl;
    std::cout << " Press 'p' to save Point Cloud" << std::endl;
    std::cout << " Press 'd' to save Depth image" << std::endl;
    std::cout << " Press 'm' to switch Point Cloud format" << std::endl;
    std::cout << " Press 'n' to switch Depth format" << std::endl;
}
