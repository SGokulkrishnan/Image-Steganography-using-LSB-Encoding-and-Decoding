#include <stdio.h>
#include<string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/*Fucntion to read and validate the arguments */
Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo)
{
    /*If strlen of argv[2] is greater 4 and the argument is .txt extension then procced further */
    if( (strlen(argv[2])>4) && strcmp(strstr(argv[2],"."),".bmp")==0)
    {
        /*Save argv[2] into structure variable*/
        decInfo->stego_image_fname = argv[2];
    }
    else
        return e_failure;

    /*If the argv[3] is given then update it in structure vaiable else provide default name*/
    if(argv[3]!=NULL)
    {
        if( strcmp(strstr(argv[3],"."),".txt")==0)
        {
            decInfo->secret_fname = argv[3];
        }
        else
            return e_failure;
    }
    else
        decInfo-> secret_fname="decoded_data.txt"; 
    return e_success;
}

/*Sub function to decode the files*/
Status open_decode_files(DecodeInfo *decInfo)
{

    /*Open the file in read mode*/
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }

    /*Open the file in write mode*/
    decInfo->fptr_secret= fopen(decInfo->secret_fname,"w");


    // Do error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }
    return e_success;
}

/*Sub function to decode the magic string*/
Status decode_magic_string( char *magic_string, DecodeInfo *decInfo)
{

    /*Seek to 54 th byte to fptr_stego image*/
    fseek(decInfo->fptr_stego_image,54,SEEK_SET);

    /*Call the decode data from image and its return e_success then return same else return e_failure*/
    if(decode_data_from_image ( magic_string , strlen ( magic_string ) , decInfo->fptr_stego_image,decInfo)==e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}


/*Sub function to decode data from the image*/
Status decode_data_from_image(char *data, int size, FILE *fptr_stego_image,DecodeInfo *decInfo)
{
    char buff[8];

    /*Run the for for size times*/
    for(int i=0;i<size;i++)
    {

        /*Store 8 byets data from fptr_stego_image to buff*/
        fread(buff,sizeof(char),8,fptr_stego_image);

        /*Call the decode_byte_to_lsb function anf if it returns e_sucess then continue else return e_failure*/
        if(decode_byte_to_lsb(data,buff,i)==e_success)
        {
            continue;
        }
        else
        {
            return e_failure;
        }
    }
    return e_success;

}

/*Sub funtion to decode_byte_to_lsb */
Status decode_byte_to_lsb( char *data , char *image_buffer,int index)
{
    char ch=0x00;

    /*Run the for loop for 8 times to decode the LSB bit and store into ch*/
    for (int i=0;i<8;i++)
    {
        ch=ch | ((image_buffer[i] & 0x01) << (7-i));   
    }

    /*If the decoded character it same as data[index] then return e_success else return e_failure*/
    if(data[index]==ch)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}


/*Sub function to decode secret file extnsize*/
Status decode_secret_file_extn_size(FILE *fptr_stego_image,DecodeInfo *decInfo)
{
    char str[32];

    /*read the 32 byte value from fptr_stego_image into str*/
    fread(str,sizeof(char),32,fptr_stego_image);

    /*Call the function decode the size if it return e_success tthen return same else return e_failure*/
    if((decode_extn_size_to_lsb(str,decInfo))==e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/*Sub fubction to decode secret file size*/
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char str[32];

    /*Read the 32 byte data from fptr_stego_image*/
    fread(str,sizeof(char),32,decInfo->fptr_stego_image);

    /*Call the sub function to decode the secret file size*/
    decode_scrt_size_to_lsb(str,decInfo);
    return e_success;
}


/*Sub fucnction to decode secret file size to lsb*/
Status decode_scrt_size_to_lsb(char *str,DecodeInfo *decInfo)
{
    int size=0;

    /*Run the forloop 32 times o decode*/
    for(int i=0;i<32;i++)
    {
        /*take LSB value from str[i] and store into masl*/
        int mask=(str[i] & 0x01);

        /*leftshift mask for 31-i times and do or operation with size*/
        size=size|mask<<(31-i);
    }

    /*Update size into size of secret file*/
    decInfo->size_secret_file=size;
    return e_success;
}

/*Sub function to decode the exten size to lsb*/
Status decode_extn_size_to_lsb(char *str,DecodeInfo *decInfo)
{
    int size=0;

    /*Run foorloop for 32 times*/
    for(int i=0;i<32;i++)
    {
        /*Take str[i] lsb value and sore into mask*/
        int mask=(str[i] & 0x01);

        /*Lef shift 31-i times and fo OR operation with size and store it in size*/
        size=size|mask<<(31-i);
    }

    /*update size value into size extn file*/
    decInfo->size_extn_file=size;
    return e_success;
}

/*Sub function to decode the secret file extention*/
Status decode_secret_file_extn(DecodeInfo *decInfo,int size)
{
    char str[8];
    for(int i=0;i<size;i++)
    {
        /*Read 8 bytes of memory from fptr_stego_image and store into str*/
        fread(str,sizeof(char),8,decInfo->fptr_stego_image);

        /*Call decode extern format*/
        decode_extn_format(str,decInfo,i);
    }
    return e_success;
}

/*Function to decode extn format*/
Status decode_extn_format(char* str,DecodeInfo *decInfo,int index)
{
    char ch=0x00;

    /*Run for loop for 8 times*/
    for (int i=0;i<8;i++)
    {
        /*get LSB and left shift 7-i after fo OR operation with ch and cupdate in ch*/
        ch=ch | ((str[i] & 0x01) << (7-i));   
    }
    /*Update ch value into extn_secret_file*/
    decInfo->extn_secret_file[index]=ch;
}

/*Sub function decode secret file extn*/
Status decode_secret_file_data(DecodeInfo *decInfo,int size)
{
    char str[8];

    /*Run forloop for size times*/
    for(int i=0;i<size;i++)
    {

        /*Read the 8 byte data from fptr_stego_image and store into str*/
        fread(str,sizeof(char),8,decInfo->fptr_stego_image);

        /*Functoin call to decode secret data*/
        decode_secret_data(str,i,decInfo);
    }
    return e_success;
}

/*Sub function to decode secret data*/
Status decode_secret_data(char *str,int index,DecodeInfo *decInfo)
{
    char ch=0x00;

    /*Run forloop to 8 times*/
    for (int i=0;i<8;i++)
    {
        /*Take LSB value from str[i] and right shift 7-i times and store into ch*/
        ch=ch | ((str[i] & 0x01) << (7-i));   
    }

    /*Update ch value into fptr_secret*/
    fputc(ch,decInfo->fptr_secret);
    return e_success;
}

/*Sub function to do decoding */
Status do_decoding(DecodeInfo *decInfo)
{
    /*Function call if it returns e_success then proceed further\n*/
    if(open_decode_files(decInfo)==e_success)
    {
        printf("Files are open successfully\n");
        /*if decode magic string is returns e_success then proceed further\n*/
        if(decode_magic_string(MAGIC_STRING,decInfo)==e_success)
        {
            printf("Decoding the magic string is successful\n");
            /*if decode_secret_file_data is returns e_success then proceed further\n*/
            if(decode_secret_file_extn_size(decInfo->fptr_stego_image,decInfo)==e_success)
            {
                printf("Decode size of secret file is successfully completed\n");
                /*if decode_secret_file_extn is returns e_success then proceed further\n*/
                if(decode_secret_file_extn(decInfo,decInfo->size_extn_file)==e_success)
                {
                    printf("Decode file format is succcessfully completed\n");
                    /*if decode_secret_file_size is returns e_success then proceed further\n*/
                    if(decode_secret_file_size(decInfo)==e_success)
                    {
                        printf("Decode secret file size is successfully completed\n");
                        /*if decode_secret_file_data is returns e_success then proceed further\n*/
                        if(decode_secret_file_data(decInfo,decInfo->size_secret_file)==e_success)
                        {
                            printf("Decode data has been completed successfully! \n");
                        }
                        else
                        {
                            printf("Decode data has been failure\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Decode secret file size failure\n");

                        return e_failure;
                    }
                }
                else
                {
                    printf("Decode file format is failure\n");
                    return e_failure;

                }
            }
            else
            {
                printf("Decode size of secret file is failure\n");
                return e_failure;
            }
        }
        else
        {
            printf("Decoding the magic string is failure\n");
            return e_failure;
        }			
    }
    else
    {
        printf("Files can't be opened\n");
        return e_failure;
    }
    return e_success;
}





















