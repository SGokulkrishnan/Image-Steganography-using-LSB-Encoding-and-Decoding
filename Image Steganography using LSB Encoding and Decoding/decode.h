#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * decoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Decode Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;

    /* Extn file info */
    long size_extn_file;
    char extn_secret_file[MAX_FILE_SUFFIX];

} DecodeInfo;

/* Decoding function prototype */

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* Store Magic String */
Status decode_magic_string( char *magic_string, DecodeInfo *decInfo);

/*decode secret file size*/
Status decode_secret_file_size(DecodeInfo *decInfo);

/* decoding the secret data*/
Status decode_secret_data(char *str,int index,DecodeInfo *decInfo);

/*decode secret size from lsb*/
Status decode_scrt_size_to_lsb(char *str,DecodeInfo *decInfo);

/*decode the extern format from file*/
Status decode_extn_format(char *str,DecodeInfo *decInfo,int i);

/* Check operation type */
OperationType check_operation_type(char *argv[]);

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/*decode extern size to lsb*/
Status decode_extn_size_to_lsb(char *str,DecodeInfo *decInfo);

/* decode function, which does the real decoding */
Status decode_data_from_image(char *data, int size, FILE *fptr_stego_image,DecodeInfo *decInfo);

/* decode a byte into LSB of image data array */
Status decode_byte_to_lsb(char *data,char *image_buffer,int index);

#endif
