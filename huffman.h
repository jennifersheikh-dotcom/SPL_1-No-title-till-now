#ifndef HUFFMAN_H
#define HUFFMAN_H

int huffman_compress_file(const char *input_path,
                          const char *output_path);

int huffman_decompress_file(const char *input_path,
                            const char *output_path);

#endif


