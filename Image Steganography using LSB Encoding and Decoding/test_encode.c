#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include "decode.h"

int main(int argc, char * argv[])
{
    //find the operation type
    if(check_operation_type(argv)==e_encode)
    {
        if(argc>=4)
        {
            printf("Selected for encoding\n"); 
            EncodeInfo encInfo;
            //read and validate encode arguments if its returns e_success then proceed futher
            if((read_and_validate_encode_args(argv,&encInfo))==e_success)
            {
                printf("Read and validate is sucessfull\n");

                //encode the data
                if(do_encoding(&encInfo)==e_success)
                {
                    printf("Encoding is sucessfully completed\n");
                }
                else
                {
                    printf("Encoding is failure\n");
                }
            }
            else
                printf("Read and validate is failure\n");
        }
        else
            printf("ERROR: Please provide atlest 4 arguments for encoding\n");
    }
   // if the opertion type is decode the proceed below
    else if(check_operation_type(argv)==e_decode)
    {
        if(argc>=3)
        {
            printf("Selectd for deconding\n");
            DecodeInfo decInfo;

            //read and validate the decode arguments
            if((read_and_validate_decode_args(argv,&decInfo))==e_success)
            {
                printf("Read and validate is sucessfull\n");

                //Call decoding part
                if(do_decoding(&decInfo)==e_success)
                {
                    printf("Decoding is sucessfully completed\n");
                }
                else
                {
                    printf("Decoding is failure\n");
                }
            }
            else
            {
                printf("Read and validate is failure\n");
            }
        }
        else
            printf("ERROR: Please provide atleasr 3 arguments for decoding\n");
    }
    else
        printf("ERROR:Invalid option\nPlease pass the valid option : -e or -d\n");
}


/*Check operation type*/
OperationType check_operation_type(char *argv[])
{
    //return the values accoring to the command_line input
    if((strcmp(argv[1],"-e"))==0)
        return e_encode;
    else if(strcmp(argv[1],"-d")==0)
        return e_decode;
    else 
        return e_unsupported;
}

