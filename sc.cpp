
#include "sc.h"

using namespace cv;
using namespace std;


bool seam_carving(Mat& in_image, int new_width, int new_height, Mat& out_image){

    // some sanity checks
    // Check 1 -> new_width <= in_image.cols
    if(new_width>in_image.cols){
        cout<<"Invalid request!!! new_width has to be smaller than the current size!"<<endl;
        return false;
    }
    if(new_height>in_image.rows){
        cout<<"Invalid request!!! ne_height has to be smaller than the current size!"<<endl;
        return false;
    }
    
    if(new_width<=0){
        cout<<"Invalid request!!! new_width has to be positive!"<<endl;
        return false;

    }
    
    if(new_height<=0){
        cout<<"Invalid request!!! new_height has to be positive!"<<endl;
        return false;
        
    }

    Mat iimage = in_image.clone();
    Mat oimage = in_image.clone();
    while(iimage.rows!=new_height || iimage.cols!=new_width){
        
        // horizontal seam if needed
        if(iimage.rows>new_height){
            
            reduce_horizontal_seam_trivial(iimage, oimage);

            /*
                //rotate the image by 90 degrees
                rotate(iimage,iimage,ROTATE_90_CLOCKWISE);

                //apply vertical seam carving method 
                reduce_vertical_seam_trivial(iimage, oimage);

                //rotate it back to original configuration
                rotate(oimage,oimage,ROTATE_90_COUNTERCLOCKWISE);
            */

            iimage = oimage.clone();
        }
        
        

        if(iimage.cols>new_width){
            reduce_vertical_seam_trivial(iimage, oimage);
            iimage = oimage.clone();
        }

        cout<<in_image.rows-iimage.rows+in_image.cols-iimage.cols<<" out of "<<in_image.cols-new_width+in_image.rows-new_height<<" seams removed"<<endl;

    }
    cout<<endl<<in_image.rows-iimage.rows<<" out of "<<in_image.rows-new_height<<" horizontal seams removed"<<endl;
    cout<<in_image.cols-iimage.cols<<" out of "<<in_image.cols-new_width<<" vertical seams removed"<<endl<<endl;
    out_image = oimage.clone();
    
    return true;
}



// vertical trivial seam is a seam through the center of the image
bool reduce_vertical_seam_trivial(Mat& in_image, Mat& out_image){
    
    // retrieve the dimensions of the new image
    int rows = in_image.rows;
    int cols = in_image.cols;
    
    // create an image slighly smaller
    out_image = Mat(rows, cols-1, CV_8UC3);

    //coverting the input image to grayscale
    Mat gray;
    cvtColor(in_image, gray, CV_BGR2GRAY);

    //applying sobel filter
    Mat sobelgrey;
    Mat dx, dy;
    //calculating X-Gradient
    Sobel(gray, dx, CV_64F, 1, 0);
    
    //calculating Y-Gradient
    Sobel(gray, dy, CV_64F, 0, 1);
    
    //Total Gradient
    magnitude(dx, dy, sobelgrey);

    //Normalizing the matrix using its maximum value (from github:: loc-trinh/seamCarving) 
    double minVal, maxVal;
    minMaxLoc(sobelgrey, &minVal, &maxVal);
    sobelgrey = sobelgrey/maxVal*255;

    //converting the image back to unsigned character
    sobelgrey.convertTo(sobelgrey, CV_8U);
  
    //creating a 2D array for storing cumulative energy values
    int CE[rows][cols];

    //storing the first row in CE similar to the energy function's first row
    for(int i=0; i<cols; i++){
        CE[0][i] = (int)sobelgrey.at<uchar>(0,i);
    }

    //filling up the cumulative energy matrix
    for(int i = 1; i < rows; i++){
        for(int j = 0; j < cols; j++){
            int temp;
            if (j == 0)
                temp = min(CE[i-1][j+1], CE[i-1][j]);
            else if (j == cols-1)
                temp = min(CE[i-1][j-1], CE[i-1][j]);
            else
                temp = min(CE[i-1][j-1], min(CE[i-1][j], CE[i-1][j+1]));
            
            CE[i][j] = (int)sobelgrey.at<uchar>(i,j) + temp;
        }
    }

    //finding the column with the least cumulative energy and storing it as the last value of the seam array
    int seam[rows];
    seam[rows-1] = -1;
    int minimum = 255*(rows+1);
    for(int j=0;j<cols;++j){
        int temp = minimum;
        minimum = min(minimum, CE[rows-1][j]);
        if(temp!=minimum){
           seam[rows-1] = j;
        }
    }

    //Backtracking to find the whole seam to be removed with the minimum enerygy value found above
    //and storing that seam in the seam array
    for(int i = rows-1; i>0; i--){
         int j = seam[i];
  
            if(seam[i]==0){
                if(CE[i-1][j+1] < CE[i-1][j])
                    seam[i-1] = j+1;
                else
                    seam[i-1] = j;   
            }
            else if(seam[i]==cols-1){
                if(CE[i-1][j-1] < CE[i-1][j])
                    seam[i-1] = j-1;
                else
                    seam[i-1] = j;
            }
            else{
                if(CE[i-1][j-1] < CE[i-1][j] && CE[i-1][j-1] < CE[i-1][j+1])
                    seam[i-1] = j-1;
                else if(CE[i-1][j+1] < CE[i-1][j] && CE[i-1][j+1] < CE[i-1][j-1])
                    seam[i-1] = j+1;
                else
                    seam[i-1] = j;
            }
    }

    //populating the output image with all but the pixels(seam) to be removed
    for(int i=0; i<rows; ++i)
    {
        int minimunIndexValue = seam[i];
        
        
            for(int j=0;j<minimunIndexValue;++j){
                Vec3b pixel = in_image.at<Vec3b>(i, j);
                
                /* at operator is r/w
                 pixel[0] --> red
                 pixel[1] --> green
                 pixel[2] --> blue
                 */
                
                
                out_image.at<Vec3b>(i,j) = in_image.at<Vec3b>(i, j);
            }
        

            for(int j=minimunIndexValue+1;j<cols;++j){
                Vec3b pixel = in_image.at<Vec3b>(i, j);
                
                /* at operator is r/w
                 pixel[0] --> red
                 pixel[1] --> green
                 pixel[2] --> blue
                 */
                
               
                out_image.at<Vec3b>(i,j-1) = pixel;
            }
    }  

    return true;
}


bool reduce_horizontal_seam_trivial(Mat& in_image, Mat& out_image){
    
    // retrieve the dimensions of the new image
    int rows = in_image.rows;
    int cols = in_image.cols;
    
    // create an image slighly smaller
    out_image = Mat(rows-1, cols, CV_8UC3);

    //coverting the input image to grayscale
    Mat gray;
    cvtColor(in_image, gray, CV_BGR2GRAY);

    //applying sobel filter
    Mat sobelgrey;
    Mat dx, dy;

    //calculating X-Gradient
    Sobel(gray, dx, CV_64F, 1, 0);
    
    //calculating Y-Gradient
    Sobel(gray, dy, CV_64F, 0, 1);
    
    //Total Gradient
    magnitude(dx, dy, sobelgrey);

    //Normalizing the matrix using its maximum value (from github:: loc-trinh/seamCarving) 
    double minVal, maxVal;
    minMaxLoc(sobelgrey, &minVal, &maxVal);
    sobelgrey = sobelgrey/maxVal*255;

    //converting the image back to unsigned character
    sobelgrey.convertTo(sobelgrey, CV_8U);
  
    //creating a 2D array for storing cumulative energy values
    int CE[rows][cols];

    //storing the first column in CE similar to the energy function's first row
    for(int i=0; i<rows; i++){
        CE[i][0] = (int)sobelgrey.at<uchar>(i,0);
    }

    //filling up the cumulative energy matrix
    for(int j = 1; j < cols; j++){
        for(int i = 0; i < rows; i++){
            int temp;
            if (i == 0)
                temp = min(CE[i+1][j-1], CE[i][j-1]);
            else if (i == rows-1)
                temp = min(CE[i-1][j-1], CE[i][j-1]);
            else
                temp = min(CE[i-1][j-1], min(CE[i][j-1], CE[i+1][j-1]));
            
            CE[i][j] = (int)sobelgrey.at<uchar>(i,j) + temp;
        }
    }

    //finding the column with the least cumulative energy and storing it as the last value of the seam array
    int seam[cols];
    seam[cols-1] = -1;
    int minimum = 255*(cols+1);
    for(int j=0;j<rows;++j){
        int temp = minimum;
        minimum = min(minimum, CE[j][cols-1]);
        if(temp!=minimum){
           seam[cols-1] = j;
        }
    }

    //Backtracking to find the whole seam to be removed with the minimum enerygy value found above
    //and storing that seam in the seam array
    for(int j = cols-1; j>0; j--){
         int i = seam[j];
  
            if(seam[j]==0){
                if(CE[i+1][j-1] < CE[i][j-1])
                    seam[j-1] = i+1;
                else
                    seam[j-1] = i;   
            }
            else if(seam[j]==rows-1){
                if(CE[i-1][j-1] < CE[i][j-1])
                    seam[j-1] = i-1;
                else
                    seam[j-1] = i;
            }
            else{
                if(CE[i-1][j-1] < CE[i][j-1] && CE[i-1][j-1] < CE[i+1][j-1])
                    seam[j-1] = i-1;
                else if(CE[i+1][j-1] < CE[i][j-1] && CE[i+1][j-1] < CE[i-1][j-1])
                    seam[j-1] = i+1;
                else
                    seam[j-1] = i;
            }
    }

    //populating the output image with all but the pixels(seam) to be removed
    for(int j=0; j<cols; ++j)
    {
        int minimunIndexValue = seam[j];
        
        
            for(int i=0;i<minimunIndexValue;++i){
                Vec3b pixel = in_image.at<Vec3b>(i, j);
                
                /* at operator is r/w
                 pixel[0] --> red
                 pixel[1] --> green
                 pixel[2] --> blue
                 */
                
                
                out_image.at<Vec3b>(i,j) = in_image.at<Vec3b>(i, j);
            }
        

            for(int i=minimunIndexValue+1;i<rows;++i){
                Vec3b pixel = in_image.at<Vec3b>(i, j);
                
                /* at operator is r/w
                 pixel[0] --> red
                 pixel[1] --> green
                 pixel[2] --> blue
                 */
                
               
                out_image.at<Vec3b>(i-1,j) = pixel;
            }
    }  

    return true;
}