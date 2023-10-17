#include <iostream>
#include <opencv2/opencv.hpp>
#include <windows.h> 
#include <chrono>

using namespace std;
using namespace cv;
using namespace std::chrono;

//class for storing images in form of 2d array.
//when init creates empty 2d array in memory.
//posible to get 3 params: width and height of image, and pointer to pixels array.
class Image {
public:
    int image_height;
    int image_width;

    unsigned char** pixels;

    Image(int h, int w) {
        image_height = h;
        image_width = w;

        pixels = new unsigned char* [image_height];
        for (int i = 0; i < image_height; i++) {
            //pixels array 3 times wider then image width due to storing r,g,b
            pixels[i] = new unsigned char[image_width * 3];
        }
    }

    ~Image() {
        for (int i = 0; i < image_height; i++) {
            delete[] pixels[i];
        }
        delete[] pixels;
    }

};

// downscaling image with nearest neighbour algorithm. 
// takes pointer to image we want to scale, new height and new width we want it to be.
// returns pointer to new image.
Image* resizeImage(Image* img, int newHeight, int newWidth) {
    if (img == nullptr || newHeight >= img->image_height || newWidth >= img->image_width) {
        return nullptr;
    }
    float scale = img->image_width / (float)newWidth;

    Image* newimg = new Image(newHeight, newWidth);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {

            newimg->pixels[y][x * 3] = img->pixels[(int)round(y * scale)][(int)round((x * 3) * scale)];
            newimg->pixels[y][x * 3 + 1] = img->pixels[(int)round(y * scale)][(int)round((x * 3 + 1) * scale)];
            newimg->pixels[y][x * 3 + 2] = img->pixels[(int)round(y * scale)][(int)round((x * 3 + 2) * scale)];

        }
    }
    return newimg;
}

//stretches image's width 2 times. Due to console characters beeing twise taller then width.
//tekes pointer to image we want to stretch.
//returns pointer to new image.
Image* stretchImage(Image* img) {
    if (img == nullptr) {
        return nullptr;
    }
    int width = img->image_width;
    int height = img->image_height;

    int newwidth = width * 2;

    Image* newimg = new Image(height, newwidth);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            newimg->pixels[y][x * 6] = img ->pixels[y][x*3];
            newimg->pixels[y][x * 6 + 1] = img->pixels[y][x * 3 + 1];
            newimg->pixels[y][x * 6 + 2] = img->pixels[y][x * 3 + 2];
            newimg->pixels[y][x * 6 + 3] = img->pixels[y][x * 3];
            newimg->pixels[y][x * 6 + 4] = img->pixels[y][x * 3 + 1];
            newimg->pixels[y][x * 6 + 5] = img->pixels[y][x * 3 + 2];
        }
    }
    return newimg;
}

//convers rgb image into grayscale
//takes pointer to image we want to grayscale
//returns pointer to new image
Image* converToGrayscale(Image* img) {
    if (img == nullptr) {
        return nullptr;
    }
    int width = img->image_width;
    int height = img->image_height;

    Image* newimg = new Image(height, width);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            unsigned char r = img->pixels[y][x * 3];
            unsigned char g = img->pixels[y][x * 3 + 1];
            unsigned char b = img->pixels[y][x * 3 + 2];

            unsigned char gray = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);

            newimg->pixels[y][x * 3] = gray;
            newimg->pixels[y][x * 3 + 1] = gray;
            newimg->pixels[y][x * 3 + 2] = gray;
        }
    }
    return newimg;
}

//converts image pixels into ascii symbols and writes them to screen array.
//takes pointer to image we want to output and pointer to screen array.
//does not output enything. Writes data directly to screen array we gave.
void show(Image* img, char* screen) {
    int height = img->image_height;
    int width = img->image_width;
 
    char gradient[] = " .:!/r(lZ4H9W8$@";
    int gradientsize = std::size(gradient) - 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int color = (img ->pixels[y][x * 3]) % gradientsize;
            char pixel = gradient[color];
            screen[x + y * width + y] = pixel;
        }
        screen[width + y * (width+1)] = '\n';
    }
}

int main(int argc, char** argv)
{
    // playspeed
    float speed = 0.8;
    //console height and width in symbols.
    int screen_width = 120;
    int screen_height = 30;
    
    int frameNum = 0;

    //imports a videofile.
    //VideoCapture video(argv[1]);
    VideoCapture video("Bad Apple!!.mp4");
    //VideoCapture video("Saul Goodman 3D Green Screen.mp4");

    /*if (argc != 2) {
        cout << "no video\n";
        return -1;
    }*/
    
    if (!video.isOpened()) {
        cout << "video cant be opened\n";
        return -1;
    }

    //gets video data: height, width, fps
    int video_width = (float)video.get(CAP_PROP_FRAME_WIDTH);
    int video_height = (float)video.get(CAP_PROP_FRAME_HEIGHT);
    double fps = video.get(CAP_PROP_FPS);

    //calculates how much we need to scale down the video to fit in console.
    float scaling = video_height / screen_height;
    int height = (int)(video_height / scaling);
    int width = (int)(video_width / scaling);

    while (true) {
        //loop start time in milliseconds. 
        uint64_t ms_start = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        Mat frame;
        video >> frame;
        if (frame.empty()) {
            break;
        }
        
        frameNum++;

        //creates and writes frame to Image class.
        Image* img = new Image(frame.rows,frame.cols);
        for (int y = 0; y < frame.rows; y++) {
            for (int x = 0; x < frame.cols; x++) {
                memcpy(img ->pixels[y],frame.ptr(y),frame.cols * sizeof(unsigned char) * 3);
            }
        }
        //applies scaling, stretching and graying.
        Image* resizedimg = resizeImage(img,height,width);
        if (resizedimg == nullptr) {
            cout << "image scaling error\n";
            return -1;
        }

        Image* srtetchedimg = stretchImage(resizedimg);
        if (srtechedimg == nullptr) {
            cout << "image streching error\n";
            return -1;
        }

        Image* grayedimg = converToGrayscale(srtetchedimg);
        if (grayedimg == nullptr) {
            cout << "image graying error\n";
            return -1;
        }

        //creates 1d array of symbols to witch will be outputed in console. 
        char* screen = new char[grayedimg->image_width * grayedimg->image_height + 1 + grayedimg->image_height];
        screen[grayedimg->image_width * grayedimg->image_height + grayedimg->image_height] = '\0';

        show(grayedimg,screen);

        //lood end time in milleseconds.
        uint64_t ms_end = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        //calculates how much we need to wait to get fps as in imported video
        uint64_t time_diff = (1000. / fps) / speed - (ms_end - ms_start);

        Sleep(time_diff);

        //prints array in console
        cout << screen;

        //deletes every array that will be re-created next frame to save memory
        delete img;
        delete resizedimg;
        delete stretchedimg;
        delete grayedimg;
        delete[] screen;
    }
    video.release();
    return 0;
}
