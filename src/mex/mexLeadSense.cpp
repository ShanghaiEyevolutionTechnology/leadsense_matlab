/*********************************
 ** Using LeadSense with Matlab **
 *********************************/

 // MEX header
#include <mex.h>
#include "matrix.h"

// OpenCV
#include <opencv2/opencv.hpp>

// Matrix format conversion by Sun Peng : http://www.mathworks.com/matlabcentral/fileexchange/20927-c-c++-and-matlab-types-convertor
#include "mc_convert.h"
#include "iter.h"
#include "types.h"

// system header
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LeadSense
#include <evo_depthcamera.h>
#include <evo_matconverter.h>

#ifdef _DEBUG
#error Select Release mode for compilation
#endif

// global var.
static evo::bino::DepthCamera *camera = NULL;

// Interop. OpenCV-Matlab matrix type (float)

template<class InputIterator, class OutputIterator>
OutputIterator fctCopy(InputIterator first, InputIterator last, OutputIterator result) {
	while (first != last) {
		*result = *first;
		++result;
		++first;
	}
	return result;
}

// Interop. OpenCV-Matlab matrix type (interal function, donot directly call this)
template<int TYPE, int channel>
mxArray* ocvmat_to_mxarray_internal(cv::Mat &image)
{
	cv::Size size = image.size();
	const mxClassID cid = cvm_traits<TYPE>::CID;
	void* pBeg = image.data;
	int pitch = image.step;

	if (channel == 1)
	{
		mxArray* pArrOut = mxCreateNumericMatrix(size.height, size.width, cid, mxREAL);
		void* pBegOut = mxGetData(pArrOut);
		typedef typename mc_traits<cid>::CT T;
		pix_iterator_2d<T, eRowWise> it_src1(static_cast<T*> (pBeg), size.width, size.height, pitch);
		pix_iterator_2d<T, eRowWise> it_src2(static_cast<T*> (pBeg), size.width, size.height, pitch);
		it_src2.end();
		pix_iterator_2d<T, eColWise> it_dest(static_cast<T*> (pBegOut), size.width, size.height);
		fctCopy(it_src1, it_src2, it_dest);
		return pArrOut;
	}
	else
	{
		const int ndim = image.channels();
		mwSize dims[3];
		dims[0] = size.height;
		dims[1] = size.width;
		dims[2] = ndim;
		mxArray* pArrOut = mxCreateNumericArray(ndim, dims, cid, mxREAL);

		void* pBegOut = mxGetData(pArrOut);

		typedef typename mc_traits<cid>::CT T;
		pix_iter_rgb<T> it_src1(static_cast<T*> (pBeg), size.width, size.height, pitch);
		pix_iter_rgb<T> it_src2(static_cast<T*> (pBeg), size.width, size.height, pitch);
		it_src2.end();
		mxArray_iter_3d<T> it_dest(static_cast<T*> (pBegOut), size.width, size.height, ndim);
		fctCopy(it_src1, it_src2, it_dest);
		return pArrOut;
	}
}


// Interop. OpenCV-Matlab matrix type (ONLY support types in the list)
mxArray* ocvmat_to_mxarray(cv::Mat &matrix) {
	const int TYPE = matrix.type();
	if (CV_32FC1 == TYPE)
		return ocvmat_to_mxarray_internal<IPL_DEPTH_32F, 1>(matrix);
	else if (CV_32FC3 == TYPE)
		return ocvmat_to_mxarray_internal<IPL_DEPTH_32F, 3>(matrix);
	else if (CV_8UC1 == TYPE)
		return ocvmat_to_mxarray_internal<IPL_DEPTH_8U, 1>(matrix);
	else if (CV_8UC3 == TYPE)
		return ocvmat_to_mxarray_internal<IPL_DEPTH_8U, 3>(matrix);

	return mxCreateDoubleMatrix(0, 0, mxREAL);
}

void deleteOnFail() {
	if (camera) {
		delete camera;
		camera = NULL;
	}
}

bool checkCamera() {
	if (camera)
		return true;
	else {
		mexErrMsgTxt("Camera is not initialized");
		deleteOnFail();
	}
	return false;
}

bool checkParams(int params, int required) {
	if (params - 1 != required) {
		std::string error = "Invalid parameter number, " + std::to_string(required) + " required, " + std::to_string(params - 1) + " given.";
		mexErrMsgTxt(error.c_str());
		deleteOnFail();
		return false;
	}
	return true;
}

bool checkParams(int params, int required_1, int required_2) {
	if ((params - 1 != required_1) && (params - 1 != required_2)) {
		std::string error = "Invalid parameter number, " + std::to_string(required_1) + " or " + std::to_string(required_2) + " required, " + std::to_string(params - 1) + " given.";
		mexErrMsgTxt(error.c_str());
		deleteOnFail();
		return false;
	}
	return true;
}

void notAvailable() {
	mexErrMsgTxt("This function isn't yet implemented on the Matlab plugin.");
	deleteOnFail();
}

/* MEX entry function */
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) 
{
	// each sub function is referred by a string 'command'
	char command[128];
	mxGetString(prhs[0], command, 128);

	if (!strcmp(command, "create")) {
		if (checkParams(nrhs, 0))
			camera = new evo::bino::DepthCamera();
	}

	else if (!strcmp(command, "open")) {
		if (checkCamera()) {
			evo::RESULT_CODE res;
			evo::bino::RESOLUTION_FPS_MODE mode = evo::bino::RESOLUTION_FPS_MODE_HD720_60;
			int camera_idx = -1;
			double *ptr_;
			int val;
			// check if we have arguments
			if(nrhs == 2) 
			{
				ptr_ = mxGetPr(prhs[1]);
				val = ptr_[0];
				if (val < evo::bino::LAST_RESOLUTION_FPS_MODE)
					mode = static_cast<evo::bino::RESOLUTION_FPS_MODE>(val);
				else {
					mexErrMsgTxt("Can't find this RESOLUTION_FPS_MODE");
					deleteOnFail();
					return;
				}				
				res = camera->open(mode);
			}
			else if (nrhs == 3)
			{
				ptr_ = mxGetPr(prhs[1]);
				val = ptr_[0];
				if (val < evo::bino::LAST_RESOLUTION_FPS_MODE)
					mode = static_cast<evo::bino::RESOLUTION_FPS_MODE>(val);
				else {
					mexErrMsgTxt("Can't find this RESOLUTION_FPS_MODE");
					deleteOnFail();
					return;
				}
				ptr_ = mxGetPr(prhs[2]);
				camera_idx = ptr_[0];
				res = camera->open(mode, camera_idx);
			}
			else if (nrhs == 4)
			{
				ptr_ = mxGetPr(prhs[1]);
				val = ptr_[0];
				if (val < evo::bino::LAST_RESOLUTION_FPS_MODE)
					mode = static_cast<evo::bino::RESOLUTION_FPS_MODE>(val);
				else {
					mexErrMsgTxt("Can't find this RESOLUTION_FPS_MODE");
					deleteOnFail();
					return;
				}
				ptr_ = mxGetPr(prhs[2]);
				camera_idx = ptr_[0];
				evo::bino::WORK_MODE work_mode = evo::bino::WORK_MODE_FAST;
				ptr_ = mxGetPr(prhs[3]);
				val = ptr_[0];
				if (val < evo::bino::LAST_WORK_MODE)
					work_mode = static_cast<evo::bino::WORK_MODE>(val);
				else {
					mexErrMsgTxt("Can't find this WORK_MODE");
					deleteOnFail();
					return;
				}
				res = camera->open(mode, camera_idx, work_mode);
			}
			else
			{
				res = camera->open();
			}

			// we return the string associated with the error
			plhs[0] = mxCreateString(evo::result_code2str(res).c_str());
		}
	}

	else if (!strcmp(command, "close")) {
		if (checkCamera() && checkParams(nrhs, 0)) {
			camera->close();
		}
	}

	else if (!strcmp(command, "isOpened")) {
		if (checkCamera() && checkParams(nrhs, 0)) {
			double val = camera->isOpened();
			plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
			memcpy(mxGetPr(plhs[0]), &val, 1 * sizeof(double));
		}
	}

	else if (!strcmp(command, "grab")) {
		if (checkCamera()) {
			evo::bino::GrabParameters grab_parameters;
			// check if there is grab parameters
			if (checkParams(nrhs, 1)) {
				// check if we have a parameter structure
				if (mxIsStruct(prhs[1])) {
					// for all fields of parameter structure overwrite parameters
					for (int32_t i = 0; i < mxGetNumberOfFields(prhs[1]); i++) {
						const char *field_name = mxGetFieldNameByNumber(prhs[1], i);
						mxArray *field_val = mxGetFieldByNumber(prhs[1], 0, i);
						int val = 0;
						char string_val[128];
						if (mxIsChar(field_val))
							mxGetString(field_val, string_val, 128);
						else
							val = *((double*)mxGetPr(field_val));
						if (!strcmp(field_name, "do_rectify")) grab_parameters.do_rectify = val;
						if (!strcmp(field_name, "depth_mode")) grab_parameters.depth_mode = static_cast<evo::bino::DEPTH_MODE> (val);
						if (!strcmp(field_name, "calc_disparity")) grab_parameters.calc_disparity = val;
						if (!strcmp(field_name, "calc_distance")) grab_parameters.calc_distance = val;
						if (!strcmp(field_name, "start_x")) grab_parameters.start_x = val;
						if (!strcmp(field_name, "start_y")) grab_parameters.start_y = val;
						if (!strcmp(field_name, "target_width")) grab_parameters.target_width = val;
						if (!strcmp(field_name, "target_height")) grab_parameters.target_height = val;
					}
				}
			}
			evo::RESULT_CODE res = camera->grab(grab_parameters);
			// we return the string associated with the error
			plhs[0] = mxCreateString(evo::result_code2str(res).c_str());
		}
	}

	else if (!strcmp(command, "retrieveImage")) {
		if (checkCamera()) {
			evo::bino::SIDE side = evo::bino::SIDE_LEFT;
			if (checkParams(nrhs, 1, 3)) {
				double *ptr_ = mxGetPr(prhs[1]);
				int val = ptr_[0];
				if (val < evo::bino::LAST_SIDE)
					side = static_cast<evo::bino::SIDE>(val);
				else {
					mexErrMsgTxt("Can't find this SIDE");
					deleteOnFail();
					return;
				}
			}

			evo::Mat<unsigned char> tmp = camera->retrieveImage(side, evo::MAT_TYPE_CPU);
			cv::Mat cv_tmp = evo::evoMat2cvMat(tmp);

			if ((tmp.getChannels() == 1) || (tmp.getChannels() == 3))
			{
				plhs[0] = ocvmat_to_mxarray(cv_tmp);
			}
			else//remove the alpha channel
			{
				cv::Mat image_rgb;
				cv::cvtColor(cv_tmp, image_rgb, CV_RGBA2RGB);
				plhs[0] = ocvmat_to_mxarray(image_rgb);
			}
		}
	}

	else if (!strcmp(command, "retrieveView")) {
		if (checkCamera()) {
			evo::bino::VIEW_TYPE view = evo::bino::VIEW_TYPE_LEFT;
			if (checkParams(nrhs, 1)) {
				double *ptr_ = mxGetPr(prhs[1]);
				int val = ptr_[0];
				if (val < evo::bino::LAST_VIEW_TYPE)
					view = static_cast<evo::bino::VIEW_TYPE>(val);
				else {
					mexErrMsgTxt("Can't support retrieve this VIEW_TYPE");
					deleteOnFail();
					return;
				}
			}

			cv::Mat image_rgb;
			evo::Mat<unsigned char> tmp = camera->retrieveView(view, evo::MAT_TYPE_CPU);
			cv::Mat cv_tmp = evo::evoMat2cvMat(tmp);
			cv::cvtColor(cv_tmp, image_rgb, CV_RGBA2RGB);
			plhs[0] = ocvmat_to_mxarray(image_rgb);
		}
	}

	else if (!strcmp(command, "retrieveDepth")) {
		if (checkCamera()) {
			evo::bino::DEPTH_TYPE depth = evo::bino::DEPTH_TYPE_DISTANCE_Z;
			if (checkParams(nrhs, 1)) {
				double *ptr_ = mxGetPr(prhs[1]);
				int val = ptr_[0];
				if (val < evo::bino::LAST_DEPTH_TYPE)
					depth = static_cast<evo::bino::DEPTH_TYPE>(val);
				else {
					mexErrMsgTxt("Can't support retrieve this DEPTH_TYPE");
					deleteOnFail();
					return;
				}
			}

			cv::Mat image_rgb;
			evo::Mat<float> tmp = camera->retrieveDepth(depth, evo::MAT_TYPE_CPU);
			cv::Mat cv_tmp = evo::evoMat2cvMat(tmp);

			if (depth == evo::bino::DEPTH_TYPE_DISTANCE_XYZ)//change xyz sequence
			{
				cv::Mat image_rgb;
				cv::cvtColor(cv_tmp, image_rgb, CV_RGBA2BGR);
				plhs[0] = ocvmat_to_mxarray(image_rgb);
			}
			else if ((tmp.getChannels() == 1) || (tmp.getChannels() == 3))
			{
				plhs[0] = ocvmat_to_mxarray(cv_tmp);
			}
			else//remove the alpha channel
			{
				cv::Mat image_rgb;
				cv::cvtColor(cv_tmp, image_rgb, CV_RGBA2RGB);
				plhs[0] = ocvmat_to_mxarray(image_rgb);
			}
		}
	}

	else if (!strcmp(command, "retrieveNormalizedDepth")) {
		if (checkCamera()) {
			evo::bino::DEPTH_TYPE depth = evo::bino::DEPTH_TYPE_DISTANCE_Z;
			if (checkParams(nrhs, 1)) {
				double *ptr_ = mxGetPr(prhs[1]);
				int val = ptr_[0];
				if (val < evo::bino::LAST_VISIABLE_DEPTH_TYPE)
					depth = static_cast<evo::bino::DEPTH_TYPE>(val);
				else {
					mexErrMsgTxt("Can't support retrieve this normalized DEPTH_TYPE");
					deleteOnFail();
					return;
				}
			}

			cv::Mat image_rgb;
			evo::Mat<unsigned char> tmp = camera->retrieveNormalizedDepth(depth, evo::MAT_TYPE_CPU);
			cv::Mat cv_tmp = evo::evoMat2cvMat(tmp);

			if ((tmp.getChannels() == 1) || (tmp.getChannels() == 3))
			{
				plhs[0] = ocvmat_to_mxarray(cv_tmp);
			}
			else//remove the alpha channel
			{
				cv::Mat image_rgb;
				cv::cvtColor(cv_tmp, image_rgb, CV_RGBA2RGB);
				plhs[0] = ocvmat_to_mxarray(image_rgb);
			}
		}
	}

	else if (!strcmp(command, "setDistanceMaxValue")) {
		if (checkCamera() && checkParams(nrhs, 1)) {
			double *ptr_ = mxGetPr(prhs[1]);
			float val = ptr_[0];
			camera->setDistanceMaxValue(val);
		}
	}

	else if (!strcmp(command, "getDistanceMaxValue")) {
		if (checkCamera() && checkParams(nrhs, 0)) {
			double val = camera->getDistanceMaxValue();
			plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
			memcpy(mxGetPr(plhs[0]), &val, 1 * sizeof(double));
		}
	}

	else if (!strcmp(command, "setConfidenceThreshold")) {
		if (checkCamera() && checkParams(nrhs, 1)) {
			double *ptr_ = mxGetPr(prhs[1]);
			float val = ptr_[0];
			camera->setConfidenceThreshold(val);
		}
	}

	else if (!strcmp(command, "getConfidenceThreshold")) {
		if (checkCamera() && checkParams(nrhs, 0)) {
			double val = camera->getConfidenceThreshold();
			plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
			memcpy(mxGetPr(plhs[0]), &val, 1 * sizeof(double));
		}
	}

	else if (!strcmp(command, "getImageSizeFPS")) {
		if (checkCamera() && checkParams(nrhs, 0)) {
			double ptr_size[3];
			ptr_size[0] = camera->getImageSizeFPS().width;
			ptr_size[1] = camera->getImageSizeFPS().height;
			ptr_size[2] = camera->getImageSizeFPS().fps;
			plhs[0] = mxCreateDoubleMatrix(3, 1, mxREAL);
			memcpy(mxGetPr(plhs[0]), ptr_size, 3 * sizeof(double));
		}
	}

	else if (!strcmp(command, "isOpened")) {
		if (checkCamera() && checkParams(nrhs, 0)) {
			double val = camera->isOpened();
			plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
			memcpy(mxGetPr(plhs[0]), &val, 1 * sizeof(double));
		}
	}

	else if (!strcmp(command, "changeWorkMode")) {
		if (checkCamera() && checkParams(nrhs, 1)) {
			evo::bino::WORK_MODE work_mode = evo::bino::WORK_MODE_FAST;
			double *ptr_ = mxGetPr(prhs[1]);
			int val = ptr_[0];
			if (val < evo::bino::LAST_WORK_MODE)
				work_mode = static_cast<evo::bino::WORK_MODE>(val);
			evo::RESULT_CODE res = camera->changeWorkMode(work_mode);
			// we return the string associated with the error
			plhs[0] = mxCreateString(evo::result_code2str(res).c_str());
		}
	}

	else if (!strcmp(command, "setAutoExposureMode")) {
		if (checkCamera() && checkParams(nrhs, 1)) {
			evo::bino::AUTO_EXPOSURE_MODE mode = evo::bino::AUTO_EXPOSURE_MODE_AVERAGE;
			double *ptr_ = mxGetPr(prhs[1]);
			int val = ptr_[0];
			if (val < evo::bino::LAST_AUTO_EXPOSURE_MODE)
				mode = static_cast<evo::bino::AUTO_EXPOSURE_MODE>(val);
			camera->setAutoExposureMode(mode);
		}
	}

	else if (!strcmp(command, "setExposureTime")) {
		if (checkCamera() && checkParams(nrhs, 1)) {
			double *ptr_ = mxGetPr(prhs[1]);
			float val = ptr_[0];
			camera->setExposureTime(val);
		}
	}

	else if (!strcmp(command, "getExposureTime")) {
		if (checkCamera() && checkParams(nrhs, 0)) {
			double val = camera->getExposureTime();
			plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
			memcpy(mxGetPr(plhs[0]), &val, 1 * sizeof(double));
		}
	}
	
	else if (!strcmp(command, "useAutoExposure")) {
		if (checkCamera() && checkParams(nrhs, 1)) {
			double *ptr_ = mxGetPr(prhs[1]);
			int val = ptr_[0];
			camera->useAutoExposure(val);
		}
	}

	else {
		mexErrMsgTxt("Can't find the specified function");
		deleteOnFail();
	}
}
