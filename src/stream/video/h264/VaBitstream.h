#ifndef SNPSERVER_VABITSTREAM_H
#define SNPSERVER_VABITSTREAM_H

#include "va/va.h"

struct __bitstream {
    unsigned int *buffer;
    int bit_offset;
    int max_size_in_dword;
};
typedef struct __bitstream bitstream;

class VaBitstream {
public:
    VaBitstream(const VAProfile &profile,
                const VAEncSequenceParameterBufferH264 &seqParam,
                const VAEncPictureParameterBufferH264 &picParam,
                const VAEncSliceParameterBufferH264 &sliceParam,
                const int &constraintSetFlag,
                const int &frameBitrate);

    virtual ~VaBitstream();

    int build_packed_pic_buffer(unsigned char **header_buffer);
    int build_packed_seq_buffer(unsigned char **header_buffer);
    int build_packed_slice_buffer(unsigned char **header_buffer);

    unsigned int va_swap32(unsigned int val);
    void bitstream_start(bitstream *bs);
    void bitstream_end(bitstream *bs);
    void bitstream_put_ui(bitstream *bs, unsigned int val, int size_in_bits);
    void bitstream_put_ue(bitstream *bs, unsigned int val);
    void bitstream_put_se(bitstream *bs, int val);
    void bitstream_byte_aligning(bitstream *bs, int bit);
    void rbsp_trailing_bits(bitstream *bs);
    void nal_start_code_prefix(bitstream *bs);
    void nal_header(bitstream *bs, int nal_ref_idc, int nal_unit_type);
    void sps_rbsp(bitstream *bs);
    void pps_rbsp(bitstream *bs);
    void slice_header(bitstream *bs);

    const VAProfile &h264_profile;
    const int &constraintSetFlag;
    const int &frameBitrate;
    const VAEncSequenceParameterBufferH264 &seqParam;
    const VAEncPictureParameterBufferH264 &picParam;
    const VAEncSliceParameterBufferH264 &sliceParam;
};

#endif //SNPSERVER_VABITSTREAM_H
