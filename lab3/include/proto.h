#ifndef _PROTO_H__
#define _PROTO_H__

#define BUFFSIZE 1024
#define DO_LIST 1
#define DO_GETFILE 2
#define DO_PUTFILE 3


typedef struct{
    int type;
    char buf[BUFFSIZE];
}MSG;

#endif
