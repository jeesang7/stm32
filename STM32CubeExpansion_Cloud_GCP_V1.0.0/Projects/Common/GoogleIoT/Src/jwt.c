/**
  ******************************************************************************
  * @file    jwt.c
  * @author  CLAB Team, MCD application team
  * @brief   JWT (JSON Web Token) helpers file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include <stdio.h>
#include <string.h>

#include "jwt.h"

#include "mbedtls/error.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/md.h"
#include "mbedtls/pk.h"
#include "net_internal.h"

#define SHA256_HASH_BITS  256
#define SHA256_HASH_BYTES (SHA256_HASH_BITS/8)

/** Validity of Json Web Token in seconds */
#define JWT_VALIDITY 10000

static char encoding_table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '-', '_'};

static int mod_table[] = {0, 2, 1};


static unsigned char 
*base64url_encode(const unsigned char *data,
                  size_t input_length,
                  size_t *output_length) 
{
  
  *output_length = 4 * ((input_length + 2) / 3);
  
  unsigned char *encoded_data = malloc(*output_length+1);
  if (encoded_data == NULL) return NULL;
  encoded_data[*output_length]='\0';
  
  for (int i = 0, j = 0; i < input_length;) {
    
    uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;
    
    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
    
    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }
  
  for (int i = 0; i < mod_table[input_length % 3]; i++)
    encoded_data[*output_length - 1 - i] = '=';
  
  return encoded_data;
}

const unsigned char RS256_header_json[]="{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
const unsigned char ES256_header_json[]="{\"alg\":\"ES256\",\"typ\":\"JWT\"}";
const char payload_json[]="{\"iat\":\"%d\",\"exp\":\"%d\",\"aud\":\"%s\"}";

int create_jwt(unsigned char *token, int token_len, int date, char *projectid, unsigned const char *key, int keylen)
{
  unsigned char temp[512];
  unsigned char *header,*payload,*signature;
  size_t len;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;
  mbedtls_pk_context pk;
  unsigned char hash[32];
  int ret;
  size_t olen; 
  const char *pers = "mbedtls_pk_sign";

  mbedtls_pk_init(&pk);
  snprintf ( (char*) temp, sizeof(temp), payload_json, date, (int) (date+JWT_VALIDITY), projectid );
  mbedtls_entropy_init( &entropy );
  mbedtls_ctr_drbg_init( &ctr_drbg );
  if( (ret = mbedtls_entropy_add_source(&entropy, mbedtls_rng_poll_cb, (void*)&hrng, 1, MBEDTLS_ENTROPY_SOURCE_STRONG)) != 0 )
  {
    printf( " failed\n  ! mbedtls_entropy_add_source returned -0x%x\n", -ret );
    return JWT_ERROR;
  }
  
  if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                                    (const unsigned char *) pers,
                                    strlen( pers ) ) ) != 0 )
  {
    return JWT_ERROR;
  }
  
  if ( (ret=mbedtls_pk_parse_key( &pk, key, keylen, NULL, (size_t) 0 )) != 0 )
  {
    return JWT_KEY_ERROR;
  }

  if( mbedtls_pk_can_do( &pk, MBEDTLS_PK_RSA ) )
  {
    header = (unsigned char*)base64url_encode(RS256_header_json,strlen((const char*)ES256_header_json),&len);
  }
  else if( mbedtls_pk_can_do( &pk, MBEDTLS_PK_ECKEY ))
  {
    header = (unsigned char*)base64url_encode(ES256_header_json,strlen((const char*)ES256_header_json),&len);
  }
  else
  {
    printf("JWT error, unsupported format key\r\n");
    return JWT_KEY_ERROR;
  }
  
  sprintf ( (char*)temp, (const char*)payload_json, date, date+JWT_VALIDITY, projectid );

  payload = (unsigned char*)base64url_encode(temp,strlen((const char*)temp),&len);

  sprintf((char*)temp,"%s.%s",header,payload);

  /* Hash and sign the header and payload */
  if( ( ret = mbedtls_md(
                         mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),
                         temp, strlen((const char*)temp), hash ) ) != 0 )
  {
    return JWT_MD_ERROR;
  }
  
  if( ( ret = mbedtls_pk_sign( &pk, MBEDTLS_MD_SHA256, hash, 0, temp, &olen,
                              mbedtls_ctr_drbg_random, &ctr_drbg ) ) != 0 )
  {
    return JWT_SIGN_ERROR;
  }
  
  /* In case of ES256 case, the signature is ASN1 encoded, whereas JWT expects raw data */
  if( mbedtls_pk_can_do( &pk, MBEDTLS_PK_ECKEY ))
  {
    unsigned char *p = temp;
    mbedtls_mpi r, s;
    const unsigned char *end = temp + olen;

    mbedtls_mpi_init( &r );
    mbedtls_mpi_init( &s );
  
    ret |= mbedtls_asn1_get_tag( &p, end, &olen,
                    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE );

    ret |= mbedtls_asn1_get_mpi( &p, end, &r );
    ret |= mbedtls_asn1_get_mpi( &p, end, &s );
    if (ret != 0)
    {
      return JWT_ERROR;
    }

    /* JWT format specifies that R and S shall be 256 bits integer */
    olen = 64;
    ret |= mbedtls_mpi_write_binary( &r, temp, 32 );
    ret |= mbedtls_mpi_write_binary( &s, temp + 32, 32);
    if( ret != 0)
    {
      return JWT_ERROR;
    }
  }

  /* finally Encode signature and concatenate the full JWT content */
  signature = base64url_encode(temp,olen,&len);
  snprintf((char*)token, token_len, "%s.%s.%s",header,payload,signature);

  free(header);
  free(payload);
  free(signature);

  return JWT_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
