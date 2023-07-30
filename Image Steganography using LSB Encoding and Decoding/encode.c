#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include <string.h>

/*Sub funtion to read and validate encode args*/
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    /*If argv[2] strlen is greater than 4 and argv[2] has .bmp then update arg[2] value into src_image_fname */
    if((strlen(argv[2])>4) && strcmp(strstr(argv[2],"."),".bmp")==0)
        encInfo -> src_image_fname = argv[2];
    else 
        return e_failure;

    /*If argv[3] strlen is greater than 4 and argv[3] has .tx then update arg[3] value into secret_fname */
    if((strlen(argv[3])>4) && strcmp(strstr(argv[3],"."),".txt")==0)
        encInfo -> secret_fname = argv[3];
    else
        return e_failure;

    /*if argv[4] not equal to null and argv[4] is.bmp file update argv[4] into stego*/
    if(argv[4]!=NULL)
    {
        if (strcmp(strstr(argv[4],"."),".bmp")==0)
            encInfo -> stego_image_fname = argv[4];
        else 
            return e_failure;
    }
    else
        encInfo -> stego_image_fname = "stego.bmp";
    return e_success;
}
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/*To find file length*/
uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}

/*Sub function to find check capacity*/
Status check_capacity(EncodeInfo *encInfo)
{
    /*find image capacity*/
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);
    /*find size of secret file*/
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);
    /*If emage cpacity is greater than my requirement the proceed further*/
    if(encInfo -> image_capacity > 16 /*magic sring size&*/ + 32 + 32 /*secret file extrn*/ +32 +(encInfo -> size_secret_file * 8) )
    {
        return e_success;
    }
    else
        return e_failure;
}

/*Sub function to find copy bmp header*/
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char str[54];

    /*Rewind the fptr sorce image*/
    rewind(fptr_src_image);

    /*copy the 1st 54 bytes from sorce image to destination image*/
    fread(str,sizeof(char),54,fptr_src_image);
    fwrite(str,sizeof(char),54,fptr_dest_image);
    return e_success;
}

/*Sub function to encode magic strinf*/
Status encode_magic_string( char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(magic_string,strlen(magic_string),encInfo -> fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

/*Sub function to encode data to image*/
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo)
{
    for(int i=0;i<size;i++) 
    {
        /*Read data from sorce image and encode from lsp and store into image data*/
        fread(encInfo -> image_data, sizeof(char),8,fptr_src_image);
        encode_byte_to_lsb(data[i],encInfo->image_data);
        fwrite(encInfo -> image_data,sizeof(char),8,encInfo->fptr_stego_image);
    }
}

/*Encode the data form LSB*/
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned int mask = 1<<7;
    for(int i=0;i<8;i++)
    {
        /*Take LSB value from data and apply mask and right shift to 7-i times and AND operation  with buffer*/
        image_buffer[i]=image_buffer[i] & 0xFE |((data & mask) >> (7-i));
        mask=mask>>1;
    }
}


/*Sub-funciton to encode secret data file extensoin size*/
Status encode_secret_file_extn_size(int size,FILE *fptr_src_image,FILE *fptr_dest_image)
{
    char str[32];

    /*store next 32 bytes from sorce image and store into str */
    fread(str,sizeof(char),32,fptr_src_image);
    /*enode soze to lsb*/
    encode_size_to_lsb(size, str);
    /*update into str*/
    fwrite(str,sizeof(char),32,fptr_dest_image);
    return e_success;
}

/*Sub function to encode the size of lsb*/
Status encode_size_to_lsb(int size,char *buffer)
{
    unsigned int mask = 1<<31;
    for(int i=0;i<32;i++)
    {
        /*encode the data */
        buffer[i]=buffer[i] & 0xFE |((size & mask) >> (31-i));
        mask=mask>>1;
    }
}

/*Sub fuction to encode the secret file ectn*/
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    /*Call function to encode to image*/
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image,encInfo);
    return e_success;
}

/*Sub function to encode the secret file size*/
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char str[32];

    /*take 32 bytes fro source and encode to file*/
    fread(str,sizeof(char),32,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size,str);
    fwrite(str,sizeof(char),32,encInfo->fptr_stego_image);
    return e_success;
}

/*Sub function to encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char secret_buff[encInfo->size_secret_file];
    rewind(encInfo->fptr_secret);

    /*Encode the the secret file data*/
    fread(secret_buff,sizeof(char),encInfo->size_secret_file,encInfo->fptr_secret);
    encode_data_to_image(secret_buff, strlen(secret_buff),encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

/*Copy remaing data from source image*/
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch,sizeof(char),1,fptr_src)>0)
        fwrite(&ch, sizeof(char),1,fptr_dest);
    return e_success;
}

/*Opertion to encode the data*/
Status do_encoding(EncodeInfo *encInfo)
{

    //if all file are successfully opned the procced
    if(open_files(encInfo)==e_success)
    {
        printf("Files are opened successfully\n");
        //check cpacity
        if(check_capacity(encInfo) == e_success)
        {
            printf("Check capacity is successfull\n");
            //copy bmp header
            if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_success)
            {
                printf("Copy bmp header is successfuly completed\n");
                //enocde the magic string
                if((encode_magic_string(MAGIC_STRING, encInfo))==e_success)
                {
                    printf("Encode the magic string is successfully completed\n");
                    strcpy(encInfo->extn_secret_file,strstr(encInfo ->secret_fname,"."));

                    if( (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo->fptr_src_image, encInfo -> fptr_stego_image)) == e_success)
                    {
                        printf("Encode size of secret file is successfully completed\n");
                        //encode secret file extension
                        if(encode_secret_file_extn(encInfo -> extn_secret_file, encInfo) == e_success)
                        {
                            printf("Encode secret extension is successfully completed\n");
                            //encode secret file size
                            if( encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_success)
                            {
                                printf("Encode secret file size is successfully completed\n");
                                //emcode secret file data
                                if(encode_secret_file_data(encInfo)==e_success)
                                {
                                    printf("Encode secret data is successfully completed\n");
                                    //copy remaining image data
                                    if(copy_remaining_img_data( encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_success)
                                    {
                                        printf("remaining data is successfully completed\n");
                                    }
                                    else
                                    {

                                        printf("remaining data is failure\n");
                                    }
                                }
                                else
                                {
                                    printf("Encode secret data is failure\n");
                                }
                            }
                            else
                            {
                                printf("Encode secret file size is failure\n");
                            }
                        }
                        else
                        {
                            printf("Encode secret extension is failure\n");
                        }
                    }
                    else
                    {
                        printf("Encode size of secret file is failure\n");
                    }
                }
                else
                {

                    printf("Encode the magic string is failure\n");
                }
            }
            else 
            {
                printf("Copy bmp header is faiure\n");
            }
        }
        else 
        {
            printf("Check capacity is failure\n");
        }
    }
    else 
    {
        printf("Open files function failure\n");
    }
    return e_success;
}
