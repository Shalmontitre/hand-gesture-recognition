#include <iostream>
#include <cv.h>
#include <highgui.h>
#define pii pair<int,int>
#define PI 3.14
using namespace cv; 
using namespace std;
char key;
Mat src,bkgrnd,rawsrc, src_gray,fore;
int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
void thresh_callback(int, void* );
void contour(Mat);
void source();
void Hull(int, void*,Mat );
//vector<int> HistR(256, 0 ),HistB(256 ,0),HistG(256,0);


int main(int argc, char** argv)
{
	cvNamedWindow("Camera_Output", 1);    //Create window
	VideoCapture cap(0);  //Capture using any camera connected to your system
	cap.read(bkgrnd);
	cvWaitKey(3000);
	while(1){ //Create infinte loop for live streaming

		cap.read(rawsrc); //Create image frames from capture
		imshow("Camera_Output", rawsrc);   //Show image frames on created window
		key = cvWaitKey(10);     //Capture Keyboard stroke
		source();
		if (char(key) == 27){
			break;      //If you hit ESC key loop will break.
		}
	}
 //Release capture.
	//cvDestroyWindow("Camera_Output"); //Destroy Window*/

//	src = imread(argv[1]);
//	contour();

	return 0;
}
//int r1,r2,c1,c2;
void source(){                                                                 // Function which finds foreground
//	Mat temp= bkgrnd.clone();
	int r1=bkgrnd.rows-1,r2=0,c1=bkgrnd.cols-1,c2=0;
	fore = Mat::zeros(rawsrc.cols,rawsrc.rows,rawsrc.type());
	Mat fore_gray;
	Vec3b p;p[0]=p[1]=p[2]=200;
	for(int i =0; i< bkgrnd.rows; i++)
		for(int j =0; j<bkgrnd.cols; j++){
			Vec3b in1 = bkgrnd.at<Vec3b>(i, j);
			Vec3b in2 = rawsrc.at<Vec3b>(i,j);
			if(abs(in1[0]-in2[0])>=50||abs(in1[1]-in2[1])>=50||abs(in1[2]-in2[2])>=50){
				/*r1 = min(r1,i);
				c1 = min(c1,j);
				r2 = max(r2,i);
				c2 = max(c2,j);*/
				fore.at<Vec3b>(Point(j,i)) = (Vec3b)p;
				}
		}
	imshow("Fore", fore);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
//	cout<<"Not Aborted"<<endl;
	cvtColor( fore, fore_gray, COLOR_BGR2GRAY );
	findContours( fore_gray, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	int Max =0, k =-1; 
	for(int i =0; i< contours.size(); i++){
		if(contours[i].size()>Max){
			k =i;
			Max = contours[i].size();
		}
	}
	if(k==-1)return;
//	cout<<"Not aborted1"<<" k is "<<k<<endl;
	for(vector<Point>:: iterator it =contours[k].begin(); it<contours[k].end();it++){
//		cout<<it->x<<it->y<<endl;
		r1 = min(it->x,r1);
		r2 = max(it->x,r2);
		c1 = min(it->y,c1);
		c2 = max(it->y,c2);
	}
  // cout <<"No abort\n"<<endl;
	if(r1>r2|| c1>c2|| r1<0||c1<0)return;
	r1 = max(r1-15,0) ; r2 = min(r2+15,rawsrc.rows-1); c1 = max(c1-15,0); c2 = min(c2+15,rawsrc.cols-1); 
	Rect myroi(c1,r1,c2-c1,r2-r1);
	Range rowr(r1,r2),colr(c1,c2);
//	cout<<"Not aborted2"<<endl;
//	src = rawsrc(myroi);
//	cout<< "r1 r2 c1 c2 is"<<r1<<' '<<r2<< ' '<<c1<<' '<<c2<<'\n'<<"R = C= "<<rawsrc.rows<<rawsrc.cols<<endl;
 //  if(c2-c1>=30&&r2-r1>=30)

	Mat drawing = Mat::zeros( rawsrc.size(), CV_8UC3 );
 for( int i = 0; i< contours.size(); i++) {
	Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
 }
		contour(rawsrc(rowr,colr));
	namedWindow("The source",WINDOW_AUTOSIZE);
	 imshow("The source", rawsrc(rowr,colr));
	 imshow("The bigcontour", drawing);

//	cout<<"Not aborted\n";
}

void contour(Mat src )
{
//	src = imread(argv[1]);
	if (src.empty())
	{
		cerr << "No image supplied ..." << endl;
		return ;
	}
	cvtColor( src, src_gray, COLOR_BGR2GRAY );
	blur( src_gray, src_gray, Size(3,3) );
//	const char* source_window = "Source";
//	namedWindow( source_window, WINDOW_AUTOSIZE );
//	imshow( source_window, src );

//	createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, Hull );
//	thresh_callback( 0, 0 );
	Hull(0,0,src);
  // waitKey(0);
	return;
}

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

float distanceP2P(Point a, Point b){
	float d= sqrt(fabs( pow(a.x-b.x,2) + pow(a.y-b.y,2) )) ;  
	return d;
}

bool Condn(Point s, Point e, Point f ){
	float l1 = distanceP2P(f,s);
	float l2 = distanceP2P(f,e);
	float dot=(s.x-f.x)*(e.x-f.x) + (s.y-f.y)*(e.y-f.y);
	float angle = acos(dot/(l1*l2));
	angle=angle*180/PI;
	return (angle<80);
}

void Hull(int, void*,Mat src )
{  
	cout<<"In Hull"<<endl;
	Mat src_copy = src.clone();
	Mat threshold_output,hsv_image,hsv_mask;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	
	Scalar  hsv_min = Scalar(0, 0, 0, 0);                                   // HSV filter
	Scalar  hsv_max = Scalar(30, 255, 255, 0);
   cvtColor(src, hsv_image, CV_BGR2HSV);
	inRange (hsv_image, hsv_min, hsv_max, threshold_output);
   inRange (hsv_image, Scalar(150,0,0,0), Scalar(180, 255,255,0 ), hsv_mask);
	add(hsv_mask, threshold_output, threshold_output);
	
/*	int dilation_size = 1;

   erode(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));

   erode(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));
   erode(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));
	
   erode(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));
   erode(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));
	dilate(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));

   dilate(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));

   dilate(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));*/
	
   /*erode(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));

   erode(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));
   erode(threshold_output,threshold_output,getStructuringElement(MORPH_RECT,Size(2*dilation_size+1,2*dilation_size+1),
			 Point(dilation_size,dilation_size)));*/
	// Detect edges using Threshold
//   threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );
 //  namedWindow("threshold aftr just hsv",WINDOW_AUTOSIZE);
	
	imshow("theshhold aftr just hsv", threshold_output);
	/// Find contours
 //  cout<<"Not aborted"<<endl;
	vector<int> HistR(256, 0 ),HistB(256 ,0),HistG(256,0);
	Mat Modified = Mat::zeros(src.rows, src.cols, src.type());
	for(int i = 0; i < src.rows; ++i)
	{
	   Vec3b	*p = src.ptr<Vec3b>(i);
		for (int j = 0; j < src.cols; ++j)
		{
	        if(threshold_output.at<uchar>(i,j)){//&&i>r1&&j>c1&&j<c2&&i<r2){                        // change for ROI
				   HistR[ p[j][2]] += 1;
					HistB[p[j][0] ] += 1;
					HistG[p[j][1]] += 1;                              // Calculating Histogram for Points inside hbv ilter
		        Modified.at<Vec3b>(Point(j,i)) = (Vec3b)p[j];          // This is giving Segfault Not working
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
  // cout<<" In hull mid"<<endl;
	uchar chB = CalcRange(HistB),chG = CalcRange(HistG), chR = CalcRange(HistR);
	 threshold_output = Mat::zeros(src.rows, src.cols, src.type()); 
   inRange(Modified, Scalar(chB-50, chG-50, chR-50), Scalar(chB+50, chG+50, chR+50), threshold_output);

//   namedWindow("threshold",WINDOW_AUTOSIZE);
//	imshow("theshhold aftr both", threshold_output);

	findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	int max =0, k =-1; 
	for(int i =0; i< contours.size(); i++){
		if(contours[i].size()>max){
			k =i;
			max = contours[i].size();
		}
	}
	if(k==-1){cout<<"error"<<endl;return ;}
//	cout<<"Mid2"<<endl;
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
					if(pointPolygonTest(hull[k], contours[k][(*it)[2]], true)>30 && Condn(contours[k][(*it)[0]],contours[k][(*it)[1]], contours[k][(*it)[2]]))
					{
						//if(pointPolygonTest(InputArray contour, Point2f pt, bool measureDist) >= 2)
						count++;
				circle( src_copy,contours[k][(*it)[2]],2,Scalar( 0, 0, 255),4 );
				circle( src_copy,contours[k][(*it)[0]],2,Scalar( 0, 255, 0),4 );
				circle( src_copy,contours[k][(*it)[1]],2,Scalar( 0, 255, 0),4 );
					}
			}
			if(wait%20==0&&count){
			if(count >= 3)
				//if(gestType != 1) 
				{
					cout<< "You are showing palm"<<endl;
					gestType = 1;
				}
			else
				//if(gestType != 0)
				if(count<2)
			{
				 cout<<"You are showing 2 fingers!" <<endl;
				 gestType = 0;
				}
				else cout<<"You are showing 3 fingers!"<<endl;
			
			}
			wait++;
	}}
  // cout<<"Mid3"<<endl;
/// Draw contours + hull results

//	Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
//	for( int i = 0; i< contours.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
  //		drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
		drawContours( src_copy, hull, k, color, 1, 8, vector<Vec4i>(), 0, Point() );

	}

	/// Show in a window
	namedWindow( "Hull demo", CV_WINDOW_AUTOSIZE );
	imshow( "Hull demo", src_copy );
	//cout<<"outta hull"<<endl;
}
