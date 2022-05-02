#ifndef __GVC_MAIN_TEST_H
#define __GVC_MAIN_TEST_H


void write_schedule(uint8_t * buffer, size_t * len);
bool enc_callback(pb_ostream_t *stream, const pb_field_iter_t *field, void * const *arg);


#endif
