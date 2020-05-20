#define MAX_DIM 6
extern "C"{
void tile(const int *x_data, int *y_data, const int ysize, const int yndim, const int xndim,
    const int xshp0, const int xshp1, const int xshp2, const int xshp3, const int xshp4, const int xshp5,
    const int yshp0, const int yshp1, const int yshp2, const int yshp3, const int yshp4, const int yshp5){
#pragma HLS INTERFACE m_axi port=x_data offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=y_data offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=x_data bundle=control
#pragma HLS INTERFACE s_axilite port=y_data bundle=control

#pragma HLS INTERFACE s_axilite port=ysize bundle=control
#pragma HLS INTERFACE s_axilite port=yndim bundle=control
#pragma HLS INTERFACE s_axilite port=xndim bundle=control

#pragma HLS INTERFACE s_axilite port=xshp0 bundle=control
#pragma HLS INTERFACE s_axilite port=xshp1 bundle=control
#pragma HLS INTERFACE s_axilite port=xshp2 bundle=control
#pragma HLS INTERFACE s_axilite port=xshp3 bundle=control
#pragma HLS INTERFACE s_axilite port=xshp4 bundle=control
#pragma HLS INTERFACE s_axilite port=xshp5 bundle=control

#pragma HLS INTERFACE s_axilite port=yshp0 bundle=control
#pragma HLS INTERFACE s_axilite port=yshp1 bundle=control
#pragma HLS INTERFACE s_axilite port=yshp2 bundle=control
#pragma HLS INTERFACE s_axilite port=yshp3 bundle=control
#pragma HLS INTERFACE s_axilite port=yshp4 bundle=control
#pragma HLS INTERFACE s_axilite port=yshp5 bundle=control

#pragma HLS INTERFACE s_axilite port = return bundle = control

  const int xshape[MAX_DIM] = {xshp0, xshp1, xshp2, xshp3, xshp4, xshp5};
  const int yshape[MAX_DIM] = {yshp0, yshp1, yshp2, yshp3, yshp4, yshp5};
  for(int i = 0; i < ysize; i++){
#pragma HLS PIPELINE 
    int o_i = i, in_i = 0, shapeSize = 1;
    for(int j = xndim-1; j >= 0; j--){
      int yj = j + yndim - xndim;
      int col = o_i % yshape[yj + MAX_DIM - yndim];
      o_i /= yshape[yj + MAX_DIM - yndim];
      col = col % xshape[j + MAX_DIM - xndim];
      in_i += col * shapeSize;
      shapeSize = shapeSize * xshape[j + MAX_DIM - xndim];
    }
    y_data[i] = x_data[in_i];
  }
}
}