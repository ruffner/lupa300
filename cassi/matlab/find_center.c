/********************************************************************************/
/*                                                                              */
/* FIND_CENTER									                                */
/*	Given the uint8(logical) image X, this function locates the centroids       */
/*	of all pixel clusters of color pixel_color or if not specified,             */
/*      the minority pixel color and returns either the uint8(logical) image Y  */
/*      where each 1 is the centroid of a pixel cluster or the cooridinates is  */
/*	real vectors M(row) and N(col), or both the binary image and the            */
/*	cooridinate vectors.							                            */
/*                                                                              */
/* Synopsis:                                                                    */
/*      Y=find_center(X);                                                       */
/*              Y=output image of type uint8(logical)                           */
/*              X=input image of type double                                    */
/*										                                        */
/*	Y=find_center(X, pixel_color);						                        */
/*		pixel_color=Color of pixel whose clusters are to be identified.         */
/*										                                        */
/*	[M, N]=find_center(X, ...);						                            */
/*		M=column vector holding the row cooridinate of pixel clusters	        */
/*		N=column vector holding the col cooridinate of pixel clusters	        */
/*										                                        */
/*	[Y, M, N]=find_center(X, ...);						                        */
/*                                                                              */
/* Daniel Leo Lau                                                               */
/* Copyright April 7, 1997                                                      */
/*                                                                              */
/********************************************************************************/

#include <math.h>
#include "mex.h"

/********************************************************************************/
/*                                                 				                */
/* MAGICWAND_MN: performs the search of all neighboring pixels to the top, left,*/
/*            bottom, and right of pixel(m,n) and returns the cooridinates.     */
/*                                                                         	    */
/********************************************************************************/
int magic_wand_mn(int pixel_list_M[], int pixel_list_N[], unsigned char Y[], unsigned char X[], int M, int N, int m, int n)
{
    int r, s, t, length_pixel_list;
    int first_previous_iteration, last_previous_iteration, next_available_slot;
    unsigned char fixed_level;
    
    length_pixel_list=M*N;
    fixed_level=X[m+n*M];
    Y[m+n*M]=1;
    
    pixel_list_M[0]=m;
    pixel_list_N[0]=n;
    first_previous_iteration=0;
    last_previous_iteration=0;
    next_available_slot=1;
    while(1){
        for (r=first_previous_iteration; r<=last_previous_iteration; r++){
            s=pixel_list_M[r]-1; t=pixel_list_N[r];
            if (s>=0 && Y[s+t*M]!=1 && (fixed_level==X[s+t*M])){
                pixel_list_M[next_available_slot]=s;
                pixel_list_N[next_available_slot]=t;
                Y[s+t*M]=1;
                next_available_slot++;
                if (next_available_slot==length_pixel_list) break;
            }
            s=pixel_list_M[r]; t=pixel_list_N[r]-1;
            if (t>=0 && Y[s+t*M]!=1 && (fixed_level==X[s+t*M])){
                pixel_list_M[next_available_slot]=s;
                pixel_list_N[next_available_slot]=t;
                Y[s+t*M]=1;
                next_available_slot++;
                if (next_available_slot==length_pixel_list) break;
            }
            s=pixel_list_M[r]+1; t=pixel_list_N[r];
            if (s<M && Y[s+t*M]!=1 && (fixed_level==X[s+t*M])){
                pixel_list_M[next_available_slot]=s;
                pixel_list_N[next_available_slot]=t;
                Y[s+t*M]=1;
                next_available_slot++;
                if (next_available_slot==length_pixel_list) break;
            }
            s=pixel_list_M[r]; t=pixel_list_N[r]+1;
            if (t<N && Y[s+t*M]!=1 && (fixed_level==X[s+t*M])){
                pixel_list_M[next_available_slot]=s;
                pixel_list_N[next_available_slot]=t;
                Y[s+t*M]=1;
                next_available_slot++;
                if (next_available_slot==length_pixel_list) break;
            }
        }
        if (last_previous_iteration==next_available_slot-1) break;
        first_previous_iteration=last_previous_iteration+1;
        last_previous_iteration=next_available_slot-1;
    }
    return(next_available_slot);
}

/********************************************************************************/
/*                                                 				                */
/* FIND_CENTER_COOR:Using magicwand_mn, this function identifies all clusters   */
/*                  and calculates their centroid, storing the results in       */
/*                  vectors centriod_coor_m and centriod_coor_n.		        */
/*                                                                         	    */
/********************************************************************************/
int find_center_coor(double centriod_coor_l[], double centriod_coor_m[], double centriod_coor_n[], unsigned char X[], unsigned char pixel_color, int M, int N)
{
    int *coor_m, *coor_n, centriod_length=0;
    int m, n, r, length_coor;
    unsigned char *Y;
    
    Y=(unsigned char*)mxCalloc(M*N, sizeof(unsigned char));
    coor_m=(int*)mxCalloc(M*N, sizeof(int));
    coor_n=(int*)mxCalloc(M*N, sizeof(int));
    
    for (n=0; n<N; n++){
        for (m=0; m<M; m++){
            if (Y[m+n*M]!=1 && X[m+n*M] == pixel_color){
                length_coor = magic_wand_mn(coor_m, coor_n, Y, X, M, N, m, n);
                for (r=0; r<length_coor; r++){
                    centriod_coor_m[centriod_length] += (double)coor_m[r];
                    centriod_coor_n[centriod_length] += (double)coor_n[r];
                }
                centriod_coor_l[centriod_length] = length_coor;
                centriod_coor_m[centriod_length] = centriod_coor_m[centriod_length]/(double)length_coor+1.0;
                centriod_coor_n[centriod_length] = centriod_coor_n[centriod_length]/(double)length_coor+1.0;
                centriod_length++;
            }
        }
    }
    mxFree(coor_m);
    mxFree(coor_n);
    return(centriod_length);
}

/********************************************************************************/
/*                                                 				                */
/* FIND_CENTER_IMAGE: Given vectors coor_m and coor_n, converts the 		    */
/*		      cooridinates into a binary image where each 1 represents          */
/*		      the center of a pixel cluster.				                    */
/*                                                                         	    */
/********************************************************************************/
void find_center_image(unsigned char Y[], double coor_m[], double coor_n[], int coor_length, int M, int N)
{
    int m;
    double x, y;
    for (m=0; m<coor_length; m++){
        y = floor(coor_m[m]);
        y = y+(double)((coor_m[m]-y)>=0.5)-1;
        x = floor(coor_n[m]);
        x = x+(double)((coor_n[m]-x)>=0.5)-1;
        Y[(int)y+(int)x*M] = 1;
    }
    return;
}

/*******************************************************************************/
/* mexFUNCTION                                                                 */
/* Gateway routine for use with MATLAB.                                        */
/*******************************************************************************/
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    long long  M, N, m, n, output_dims[2];
    long number_zeros=0, number_ones=0, length_coor=0;
    unsigned char *output_data, *input_data, pixel_color;
    double *l_coor, *m_coor, *n_coor, *output_coor;
    char neighborhood[5];
    
    if (nrhs>2){
        mexErrMsgTxt("FIND_CENTER accepts one or two input argument!");
    } else if (nlhs>4) {
        mexErrMsgTxt("FIND_CENTER returns one to three output arguments!");
    } else if (!mxIsLogical(prhs[0])) {
        mexErrMsgTxt("Input X must be a real matrix of type LOGICAL!");
    } else if (nrhs==2 && (!mxIsNumeric(prhs[1]) || mxIsComplex(prhs[1]) || mxIsSparse(prhs[1]) || !mxIsDouble(prhs[1]))) {
        mexErrMsgTxt("Input pixel color must be real scalar!");
    }
    
    M=mxGetM(prhs[0]);
    N=mxGetN(prhs[0]);

    input_data=(unsigned char*)mxGetLogicals(prhs[0]);
    
    if (nrhs==2){
        pixel_color=mxGetScalar(prhs[1]);
        if (pixel_color!=1.0 && pixel_color!=0.0){
            mexErrMsgTxt("Pixel color must be one or zero!");
        }
    } else{
        for (m=0; m<M*N; m++){
            if (input_data[m]==1){
                number_ones++;
            } else {
                number_zeros++;
            }
        }
        pixel_color = (number_zeros > number_ones);
    }
    
    l_coor=(double*)mxCalloc(M*N, sizeof(double));
    m_coor=(double*)mxCalloc(M*N, sizeof(double));
    n_coor=(double*)mxCalloc(M*N, sizeof(double));

    length_coor = find_center_coor(l_coor, m_coor, n_coor, input_data, pixel_color, M, N);
    
    if (nlhs==1){
        output_dims[0]=M; output_dims[1]=N;
        plhs[0]=mxCreateLogicalArray(2, output_dims);
        output_data=mxGetLogicals(plhs[0]);
        find_center_image(output_data, m_coor, n_coor, length_coor, M, N);
    } else if (nlhs==2){
        plhs[0]=mxCreateDoubleMatrix(length_coor, 1, mxREAL);
        output_coor=mxGetPr(plhs[0]);
        for (m=0; m<length_coor; m++) {
            output_coor[m]=m_coor[m];
        }
        plhs[1]=mxCreateDoubleMatrix(length_coor, 1, mxREAL);
        output_coor=mxGetPr(plhs[1]);
        for (m=0; m<length_coor; m++) output_coor[m]=n_coor[m];
    } else if (nlhs==3){
        // SET THE OUTPUT IMAGE TO EQUAL THE SIZE OF THE INPUT IMAGE
        output_dims[0]=M;
        output_dims[1]=N;
        
        // CREATE THE LOGICAL OUTPUT IMAGE
        plhs[0]=mxCreateLogicalArray(2, output_dims);
        
        // GET THE POINTER TO THE LOGICAL IMAGE BUFFER
        output_data=mxGetLogicals(plhs[0]);
        
        // CALL THE FUNCTION TO PROCESS THE INPUT IMAGE
        find_center_image(output_data, m_coor, n_coor, length_coor, M, N);
        
        // CREATE A SECOND OUTPUT MATRIX TO HOLD THE ROW COORDINATES OF THE CLUSTER CENTROIDS
        plhs[1]=mxCreateDoubleMatrix(length_coor, 1, mxREAL);
        output_coor=mxGetPr(plhs[1]);
        for (m=0; m<length_coor; m++) {
            output_coor[m] = m_coor[m];
        }
        
        // CREATE A THIRD OUTPUT MATRIX TO HOLD THE COLUMN COORDINATES OF THE CLUSTER CENTROIDS
        plhs[2] = mxCreateDoubleMatrix(length_coor, 1, mxREAL);
        output_coor = mxGetPr(plhs[2]);
        for (m=0; m<length_coor; m++) {
            output_coor[m] = n_coor[m];
        }
    } else if (nlhs==4){
        // SET THE OUTPUT IMAGE TO EQUAL THE SIZE OF THE INPUT IMAGE
        output_dims[0]=M;
        output_dims[1]=N;
        
        // CREATE THE LOGICAL OUTPUT IMAGE
        plhs[0]=mxCreateLogicalArray(2, output_dims);
        
        // GET THE POINTER TO THE LOGICAL IMAGE BUFFER
        output_data=mxGetLogicals(plhs[0]);
        
        // CALL THE FUNCTION TO PROCESS THE INPUT IMAGE
        find_center_image(output_data, m_coor, n_coor, length_coor, M, N);
        
        // CREATE A SECOND OUTPUT MATRIX TO HOLD THE ROW COORDINATES OF THE CLUSTER CENTROIDS
        plhs[1]=mxCreateDoubleMatrix(length_coor, 1, mxREAL);
        output_coor=mxGetPr(plhs[1]);
        for (m=0; m<length_coor; m++) {
            output_coor[m] = m_coor[m];
        }
        
        // CREATE A THIRD OUTPUT MATRIX TO HOLD THE COLUMN COORDINATES OF THE CLUSTER CENTROIDS
        plhs[2] = mxCreateDoubleMatrix(length_coor, 1, mxREAL);
        output_coor = mxGetPr(plhs[2]);
        for (m=0; m<length_coor; m++) {
            output_coor[m] = n_coor[m];
        }

        // CREATE A FOURTH OUTPUT MATRIX TO HOLD THE SIZE OF THE CLUSTER CENTROIDS
        plhs[3] = mxCreateDoubleMatrix(length_coor, 1, mxREAL);
        output_coor = mxGetPr(plhs[3]);
        for (m=0; m<length_coor; m++) {
            output_coor[m] = l_coor[m];
        }
    }
    return;
}
