#ifndef _PLF_TRACE_H_
#define _PLF_TRACE_H_

void mp_2p(unsigned long long timestamp, unsigned char cnt, unsigned int *value);
void mp_2pr(unsigned long long timestamp, unsigned char cnt, unsigned int *value);
void mp_2pw(unsigned long long timestamp, unsigned char cnt, unsigned int *value);

void ms_emi(unsigned long long timestamp, unsigned char cnt, unsigned int *value);
void ms_smi(unsigned long long timestamp, unsigned char cnt, unsigned int *value);
void ms_smit(unsigned long long timestamp, unsigned char cnt, unsigned int *value);

void ms_th(unsigned long long timestamp, unsigned char cnt, unsigned int *value);

#endif // _PLF_TRACE_H_
