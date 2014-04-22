#include "fisheye.h"


FishEye::FishEye(const std::string &topicsName, const std::string &paramPath)
    : ImageHandler(topicsName)
{

    this->_init = this->_loadParam(paramPath);

    _isSampled = false;

    _sampling_ratio = 1;
}

FishEye::FishEye(const std::string &topicsName, const std::string &paramPath, const std::string &LUTSphPath)
    : ImageHandler(topicsName)
{

    this->_init = this->_loadParam(paramPath);

    this->LoadLUT(LUTSphPath,"sphere");

    _isSampled = false;

    _sampling_ratio = 1;
}


FishEye::~FishEye(){


}


bool FishEye::_loadParam(const std::string &paramPath)
{
    cv::FileStorage fs(paramPath,cv::FileStorage::READ);

    if (!fs.isOpened())
    {
        std::cout<<"Failed to open "<<paramPath<< std::endl;
        return false;
    }



    fs["type"] >> this->_cameraParam.cameraType;
    fs["xi"] >> this->_cameraParam.xi;
    fs["K"] >> this->_cameraParam.intrinParam;

    cv::FileNode fn = fs["image_size"];

    fn["rows"] >> this->_cameraParam.imSize.rows;
    fn["cols"] >> this->_cameraParam.imSize.cols;

    fs.release();

    this->_cameraParam.intrinParam.convertTo(this->_cameraParam.intrinParam,CV_32FC1);

    return true;
}

std::string FishEye::GetType(){
    return this->_cameraParam.cameraType;
}

double FishEye::GetXi(){
    return this->_cameraParam.xi;
}

cv::Mat FishEye::GetIntrinsic(){
    return this->_cameraParam.intrinParam;
}

std::vector<int> FishEye::GetImageSize(){

    std::vector<int> tmp;

    tmp.push_back(this->_cameraParam.imSize.rows);
    tmp.push_back(this->_cameraParam.imSize.cols);

    return tmp;
}

bool FishEye::IsSampled(){
    return this->_isSampled;
}

cv::Mat FishEye::GetLUT(){
    return this->_LUTsphere;
}

cv::Mat FishEye::GetLUT(const std::string &LUT ){

    std::string LUTsphere =  "Sphere";
    std::string LUTheal = "Healpix";
    std::string LUTplatte = "PlCa";

    if (LUT.compare(LUTsphere) == 0 && !this->_LUTsphere.empty())
    {

        return this->_LUTsphere;


    }else if(LUT.compare(LUTheal) == 0 && !this->_LUT_wrap_im.empty()){

        return this->_LUT_wrap_im;

    }else if(LUT.compare(LUTplatte) == 0 && !this->_LUT_wrap_im.empty()){

        return this->_LUT_wrap_im;

    }else{

        std::cout<<"Wrong LUT choice, please choose a correct option :\n"<<
                   "1 : 'Sphere' for points lying on the S2 sphere\n"<<
                   "2 : 'Healpix' for the Healpix unwrapped points\n"<<
                   "3 : 'PlCa' for Platte Carree unwrapped points"<<std::endl;

        return cv::Mat::zeros(1,1,CV_32F);
    }


}

cv::Mat FishEye::GetMask(){
    return this->_Mask;
}

void FishEye::ReadFrame(){

    sensor_msgs::ImageConstPtr frame = ImageHandler::waitUntilImageReceived();

    cv_bridge::CvImagePtr cvPtr;

    cvPtr = cv_bridge::toCvCopy(frame,"8UC3");

    this->_Frame = cvPtr->image;

    if(this->_isSampled) cv::resize(this->_Frame,this->_Frame,cv::Size(),1.0/this->_sampling_ratio,1.0/this->_sampling_ratio);
}

cv::Mat FishEye::getImage(){
    return this->_Frame;
}

bool FishEye::IsInit(){
    return this->_init;
}

void FishEye::SetType(const std::string& type){
    this->_cameraParam.cameraType = type;
}

void FishEye::SetXi(double xi){
    this->_cameraParam.xi = xi;
}

void FishEye::SetIntrinsic(const cv::Mat &intrin){
    this->_cameraParam.intrinParam = intrin;
}

void FishEye::SetImageSize(const imageSize &imSize){
    this->_cameraParam.imSize = imSize;
}

void FishEye::SetImageSize(int rows, int cols){
    this->_cameraParam.imSize.rows = rows;
    this->_cameraParam.imSize.cols = cols;
}

void FishEye::SetLUTSph(const cv::Mat &LUTSph)
{
    this->_LUTsphere = LUTSph;
}

void FishEye::ReleaseLut(){
    if (!this->_LUT_wrap_im.empty()) this->_LUT_wrap_im.release();
    if (!this->_LUTsphere.empty()) this->_LUTsphere.release();
}

void FishEye::DispParam()
{
    std::cout << "camera type : " << this->_cameraParam.cameraType << std::endl;

    std::cout << "camera xi   : " << this->_cameraParam.xi << std::endl;

    std::cout << "camera intrinsic parameters : \n " << this->_cameraParam.intrinParam << std::endl;

    std::cout << "image size :\n   rows : " << this->_cameraParam.imSize.rows
              << "\n   cols : " << this->_cameraParam.imSize.cols << std::endl<< std::endl;
}

bool FishEye::LoadLUT(const std::string& filename, const std::string &LUT)
{

    std::ifstream fs;

    fs.open ((char*)filename.c_str());

    if ( !fs.is_open() )
    {
        std::cout<<"Failed to open "<<filename<< std::endl;
        return false;
    }

    std::string LUTsphere =  "Sphere";
    std::string LUTheal = "Healpix";
    std::string LUTplatte = "PlCa";

    std::vector<float> tmp_vec;
    std::string num;

    if (LUT.compare(LUTsphere) == 0)
    {

        while(std::getline(fs,num,','))
        {

            tmp_vec.push_back(atof(num.c_str()));

        }

        this->_LUTsphere = Vector2Mat<float>(tmp_vec);

        this->_LUTsphere = this->_LUTsphere.reshape(0,tmp_vec.size()/3);

        this->_LUTsphere = this->_LUTsphere.t();


    }else if(LUT.compare(LUTheal)  == 0 ){

        while(std::getline(fs,num,','))
        {

            tmp_vec.push_back(atof(num.c_str()));

        }

        this->_LUT_wrap_im = Vector2Mat<float>(tmp_vec);

        this->_LUT_wrap_im = this->_LUT_wrap_im.reshape(0,tmp_vec.size()/2);

        this->_LUT_wrap_im = this->_LUT_wrap_im.t();

    }else if(LUT.compare(LUTplatte)  == 0 ){

        while(std::getline(fs,num,','))
        {

            tmp_vec.push_back(atof(num.c_str()));

        }

        this->_LUT_wrap_im = Vector2Mat<float>(tmp_vec);

        this->_LUT_wrap_im = this->_LUT_wrap_im.reshape(0,tmp_vec.size()/2);

        this->_LUT_wrap_im = this->_LUT_wrap_im.t();

    }else{

        std::cout<<"Error while loading LUT, please choose a correct option :\n"<<
                   "1 : "<<LUTsphere<<" for points lying on the S2 sphere\n"<<
                   "2 : "<<LUTheal<<" for the Healpix unwrapped points\n"<<
                   "3 : "<<LUTplatte<<" for Platte Carree unwrapped points"<<std::endl<<std::endl;

        return false;
    }

    fs.close();

    return true;
}



void FishEye::LoadMask(const std::string& maskFile){

    if (maskFile.empty()) return;

    cv::Mat tmp;

    tmp = cv::imread(maskFile,CV_LOAD_IMAGE_GRAYSCALE);

    cv::threshold(tmp,tmp,240,1,cv::THRESH_BINARY);

    tmp.convertTo(this->_Mask,CV_8UC1);
}

void FishEye::readImage(std::string file){

    ImageHandler::readImage(file,this->_Frame);

}

void FishEye::Im2Sph(int rows,int cols){

    if (!this->IsInit()) return;

    if (this->_Mask.empty()) this->_Mask = cv::Mat::ones(this->_cameraParam.imSize.rows,this->_cameraParam.imSize.cols,CV_8UC1);

    this->_LUTsphere = cv::Mat::zeros(3,rows * cols,CV_32FC1);

    cv::Mat pts = cv::Mat::ones(3,1,CV_32FC1);

    float alpha;

    int i = 0;

    cv::Mat inv_K = this->_cameraParam.intrinParam.inv();

    std::complex<float> tmp;

    for (int col = 0; col < cols; col++)
    {
        for (int row = 0; row < rows; row++)
        {
            if (!this->_Mask.at<uchar>(row,col) == 0)
            {
                pts.at<float>(0) = (float)col;
                pts.at<float>(1) = (float)row;

//                            std::cout << "pts : "<< pts<<std::endl;

                pts = inv_K * pts;

//                            std::cout << "K pts : "<< pts<<std::endl;

                tmp = std::sqrt( std::complex<float>(pts.at<float>(2) * pts.at<float>(2) + (1 - this->_cameraParam.xi * this->_cameraParam.xi) *
                                                     (pts.at<float>(0) * pts.at<float>(0) + pts.at<float>(1) * pts.at<float>(1))) );

                alpha = (this->_cameraParam.xi * pts.at<float>(2) + (float)tmp.real())
                        / (pts.at<float>(0) * pts.at<float>(0) + pts.at<float>(1) * pts.at<float>(1) + pts.at<float>(2) * pts.at<float>(2));


//                            std::cout << "comp conv : "<< std::complex<float>(pts.at<float>(2) * pts.at<float>(2) + (1 - this->_cameraParam.xi * this->_cameraParam.xi) *
//                                                                                    (pts.at<float>(0) * pts.at<float>(0) + pts.at<float>(1) * pts.at<float>(1))) <<std::endl;

//                            std::cout << "tmp : "<< tmp <<std::endl;
//                            std::cout << "tmp real : "<< (float)tmp.real() <<std::endl;


//                            std::cout << "alpha : "<< alpha<<std::endl;

                this->_LUTsphere.at<float>(0,i) = pts.at<float>(0) * alpha;
                this->_LUTsphere.at<float>(1,i) = pts.at<float>(1) * alpha;
                this->_LUTsphere.at<float>(2,i) = pts.at<float>(2) * alpha - this->_cameraParam.xi;
            }
            i++;
        }
    }
}

void FishEye::Im2Sph(const cv::Size &im){
    this->Im2Sph(im.height,im.width);
}



cv::Vec3f FishEye::Pix2Sph(int ind_row, int ind_col)
{
    cv::Vec3f pts;

    cv::Mat _pts = cv::Mat::ones(3,1,CV_32FC1);

    _pts.at<float>(0) = ind_col;
    _pts.at<float>(1) = ind_row;

    float alpha;

    _pts = this->_cameraParam.intrinParam.inv() * _pts;

    alpha = (this->_cameraParam.xi * _pts.at<float>(2) + sqrt(_pts.at<float>(2)*_pts.at<float>(2) + (1-this->_cameraParam.xi*this->_cameraParam.xi)*
                (_pts.at<float>(0)*_pts.at<float>(0) + _pts.at<float>(1)*_pts.at<float>(1)))) / cv::norm(_pts);

    pts[0] = _pts.at<float>(0) * alpha;
    pts[1] = _pts.at<float>(1) * alpha;
    pts[2] = _pts.at<float>(2) * alpha - this->_cameraParam.xi;

    return pts;
}



void FishEye::DownSample(int sampling_ratio)
{

    if (!this->IsInit()) return;

    this->_cameraParam.imSize.cols /= sampling_ratio;
    this->_cameraParam.imSize.rows /= sampling_ratio;

    this->_cameraParam.intrinParam /= sampling_ratio;
    this->_cameraParam.intrinParam.at<float>(2,2) = 1;

    if (!this->_Frame.empty()) cv::resize(this->_Frame,this->_Frame,cv::Size(),1.0/sampling_ratio,1.0/sampling_ratio);

    if (!this->_Mask.empty()) cv::resize(this->_Mask,this->_Mask,cv::Size(),1.0/sampling_ratio,1.0/sampling_ratio);

    if (!this->_LUTsphere.empty()) this->Im2Sph(this->_cameraParam.imSize.rows,this->_cameraParam.imSize.cols);

    this->_isSampled = true;

    this->_sampling_ratio = sampling_ratio;

//    if (!this->_LUT_wrap_im.empty()) this->_LUT_wrap_im.

}


void FishEye::Sph2Im(const cv::Mat &sphPoints)
{
    if (!this->IsInit() || sphPoints.empty()) return;

    //if (sphPoints.rows == 2) Sph2Cart(sphPoints,sphPoints);

    this->_LUT_sampSph_im = cv::Mat::zeros(2,sphPoints.cols,CV_8U);

    int row,col;

    for (int i = 0; i < sphPoints.cols; i++)
    {
        col = (int)((sphPoints.at<float>(0,i) * this->_cameraParam.intrinParam.at<float>(0,0)) /
              (sphPoints.at<float>(2,i) + this->_cameraParam.xi) + this->_cameraParam.intrinParam.at<float>(0,2));

        row = (int)((sphPoints.at<float>(1,i) * this->_cameraParam.intrinParam.at<float>(1,1)) /
                (sphPoints.at<float>(2,i) + this->_cameraParam.xi) + this->_cameraParam.intrinParam.at<float>(1,2));

        this->_LUT_sampSph_im.at<int>(0,i) = row;
        this->_LUT_sampSph_im.at<int>(1,i) = col;
    }
}

void FishEye::Sph2Im(const cv::Mat &sphPts,cv::Mat &imPts)
{
    if (!this->IsInit() || sphPts.empty()) return;

    //if (sphPts.rows == 2) Sph2Cart(sphPts,sphPts);

    imPts = cv::Mat::zeros(2,sphPts.cols,CV_32S);

    int row,col;

    for (int i = 0; i < sphPts.cols; i++)
    {
        col = (int)((sphPts.at<float>(0,i) * this->_cameraParam.intrinParam.at<float>(0,0)) /
              (sphPts.at<float>(2,i) + this->_cameraParam.xi) + this->_cameraParam.intrinParam.at<float>(0,2));

        row = (int)((sphPts.at<float>(1,i) * this->_cameraParam.intrinParam.at<float>(1,1)) /
                (sphPts.at<float>(2,i) + this->_cameraParam.xi) + this->_cameraParam.intrinParam.at<float>(1,2));

        imPts.at<int>(0,i) = row;
        imPts.at<int>(1,i) = col;
    }
}

cv::Vec3f FishEye::Sph2Im(float th, float phi)
{

}
