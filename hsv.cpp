#include <iostream>
#include <cv.h>
#include <highgui.h>
#define pii pair<int,int>
using namespace cv; 
using namespace std;
char key;
Mat src; Mat src_gray;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
void thresh_callback(int, void* );
void contour();
void Hull(int, void* );
//vector<int> HistR(256, 0 ),HistB(256 ,0),HistG(256,0);


int main(int argc, char** argv)
{
	//cvNamedWindow("Camera_Output", 1);    //Create window
	VideoCapture cap(0);  //Capture using any camera connected to your system
	while(1){ //Create infinte loop for live streaming

		cap.read(src); //Create image frames from capture
	//	imshow("Camera_Output", src);   //Show image frames on created window
		key = cvWaitKey(10);     //Capture Keyboard stroke
		if (char(key) == 27){
			break;      //If you hit ESC key loop will break.
		}
		contour();
	}
 //Release capture.
	//cvDestroyWindow("Camera_Output"); //Destroy Window*/

//	src = imread(argv[1]);
//	contour();

	return 0;
}

void contour( )
{
//	src = imread(argv[1]);
	if (src.empty())
	{
		cerr << "No image supplied ..." << endl;
		return ;
	}
	cvtColor( src, src_gray, COLOR_BGR2GRAY );
	blur( src_gray, src_gray, Size(3,3) );
	const char* source_window = "Source";
	namedWindow( source_window, WINDOW_AUTOSIZE );
	imshow( source_window, src );
//	createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, Hull );
//	thresh_callback( 0, 0 );
	Hull(0,0);
  // waitKey(0);
	return;
}
/*void thresh_callback(int, void* )
{
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Canny( src_gray, canny_output, thresh, thresh*2, 3 );
	findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
	Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
	for( size_t i = 0; i< contours.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		drawContours( drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );
	}
	namedWindow( "Contours", WINDOW_AUTOSIZE );
	imshow( "Contours", drawing );
}
*/

uchar CalcRange(vector<int> &HistE){
	int max1 =0,k= -1,sum=0; 
	for(int i =0 ; i<80; i++){
		sum+= HistE[i];
	}
	for(int i =80; i< 256; i++){
		sum+= HistE[i];
		sum-= HistE[i-80];
		if(sum >max1){
			max1 =sum;
			k = i;
		}
	}
	return k-40;
}
	

void Hull(int, void* )
{
	Mat src_copy = src.clone();
	Mat threshold_output,hsv_image,hsv_mask;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Scalar  hsv_min = Scalar(0, 0, 0, 0);
	Scalar  hsv_max = Scalar(30, 255, 255, 0);
   cvtColor(src, hsv_image, CV_BGR2HSV);
	inRange (hsv_image, hsv_min, hsv_max, threshold_output);
   inRange (hsv_image, Scalar(150,0,0,0), Scalar(180, 255,255,0 ), hsv_mask);
   add(hsv_mask, threshold_output, threshold_output);
	//inRange ( hsv_image, Scalar(150
	// Detect edges using Threshold
//   threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );
 //  namedWindow("threshold",WINDOW_AUTOSIZE);
	imshow("theshhold aftr just hsv", threshold_output);
	/// Find contours
   


	vector<int> HistR(256, 0 ),HistB(256 ,0),HistG(256,0);
	Mat Modified = Mat::zeros(src.rows, src.cols, src.type());
	for(int i = 0; i < src.rows; ++i)
	{
	   Vec3b	*p = src.ptr<Vec3b>(i);
		for (int j = 0; j < src.cols; ++j)
		{
	        if(threshold_output.at<uchar>(i,j) ){
				   HistR[ p[j][2]] += 1;
					HistB[p[j][0] ] += 1;
					HistG[p[j][1]] += 1;                              // Calculating Histogram for Points inside hbv ilter
		        // Modified.at<Vec3b>(Point(i,j)) = (Vec3b)p[j];          // This is giving Segfault Not working
			      //Vec3b *f = &Modified.at<Vec3b>(Point(i,j));
					//(*f)[0] = p[j][0];
			  }
		/*  else{	  
		         Modified.at<Vec3b>(Point(i,j)) = p[j];          // Calculating Histogram for Points inside hbv ilter
			      Vec3b *f = &Modified.at<Vec3b>(Point(i,j));
				//	cout<<"Gonna core";
					(*f) = Vec3b(0,0,0);
				//	cout<<"Not"<<endl;
		}*/
		}
	}

	uchar chB = CalcRange(HistB),chG = CalcRange(HistG), chR = CalcRange(HistR);
   inRange(src_copy, Scalar(chB-40, chG-40, chR-40), Scalar(chB+40, chG+40, chR+40), threshold_output);

//   namedWindow("threshold",WINDOW_AUTOSIZE);
	imshow("theshhold aftr both", threshold_output);

	findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	int max =0, k =0; 
	for(int i =0; i< contours.size(); i++){
		if(contours[i].size()>max){
			k =i;
			max = contours[i].size();
		}
	}
	/// Find the convex hull object for each contour
	//vector<vector<Point> >hull( contours.size() );
	vector<vector<Point> > hull( contours.size() );
	vector<vector<int> > hullsI(contours.size());
	vector<Point> FarPoints;
	//vector<Vec4i> defects;
	vector<vector<Vec4i> > defects( contours.size() );
	Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
   static int gestType=0,wait =0;;int count =0;  
//	for( int i = 0; i < contours.size(); i++ )
	{  
		convexHull( Mat(contours[k]), hull[k], false );
		convexHull( Mat(contours[k]), hullsI[k], false );
  	  if (contours[k].size() >3 )
		{
			convexityDefects(contours[k], hullsI[k], defects[k]);
				for(vector<Vec4i>::iterator it = defects[k].begin(); it<defects[k].end(); it++){
					if(pointPolygonTest(hull[k], contours[k][(*it)[2]], true)>30)
					{
						//if(pointPolygonTest(InputArray contour, Point2f pt, bool measureDist) >= 2)
						count++;
				circle( drawing,contours[k][(*it)[2]],2,Scalar( 0, 0, 255),4 );}
			}
			if(wait%20==0&&count){
			if(count >= 4)
				//if(gestType != 1) 
				{
					cout<< "You are showing palm"<<endl;
					gestType = 1;
				}
			else
				//if(gestType != 0)
			{
				 cout<<"You are showing 2 fingers!" <<endl;
				 gestType = 0;
				}
			
			}
			wait++;
	}}

/// Draw contours + hull results

//	Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
//	for( int i = 0; i< contours.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
  //		drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
		drawContours( drawing, hull, k, color, 1, 8, vector<Vec4i>(), 0, Point() );

	}

	/// Show in a window
	namedWindow( "Hull demo", CV_WINDOW_AUTOSIZE );
	imshow( "Hull demo", drawing );
}
