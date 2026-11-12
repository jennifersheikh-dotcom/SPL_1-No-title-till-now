
#include "huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <sys/stat.h>

#define SYMBOLS 256
#define MAX_NODES 511
#define MAX_CODE_LENGTH 256

static const unsigned char MAGIC[8] = {
    'M', 'E', 'O', 'W', 'H', 'U', 'F', '1'
};

/* ==================== HUFFMAN NODE ==================== */

typedef struct HuffmanNode
{
    unsigned char symbol;
    uint64_t frequency;
    int min_symbol;

    struct HuffmanNode *left;
    struct HuffmanNode *right;

} HuffmanNode;

/* ==================== MIN HEAP ==================== */

typedef struct
{
    HuffmanNode *nodes[SYMBOLS];
    int size;

} MinHeap;

static int is_leaf(const HuffmanNode *node)
{
    return node->left == NULL && node->right == NULL;
}

static int node_less(const HuffmanNode *a, const HuffmanNode *b)
{
    if (a->frequency != b->frequency)
        return a->frequency < b->frequency;

    return a->min_symbol < b->min_symbol;
}

static void heap_swap(HuffmanNode **a, HuffmanNode **b)
{
    HuffmanNode *temp = *a;
    *a = *b;
    *b = temp;
}

static void heap_push(MinHeap *heap, HuffmanNode *node)
{
    int index = heap->size++;

    heap->nodes[index] = node;

    while (index > 0)
    {
        int parent = (index - 1) / 2;

        if (!node_less(heap->nodes[index], heap->nodes[parent]))
            break;

        heap_swap(&heap->nodes[index], &heap->nodes[parent]);
        index = parent;
    }
}

static HuffmanNode *heap_pop(MinHeap *heap)
{
    if (heap->size == 0)
        return NULL;

    HuffmanNode *result = heap->nodes[0];

    heap->size--;

    if (heap->size == 0)
        return result;

    heap->nodes[0] = heap->nodes[heap->size];

    int index = 0;

    while (1)
    {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int smallest = index;

        if (left < heap->size &&
            node_less(heap->nodes[left], heap->nodes[smallest]))
        {
            smallest = left;
        }

        if (right < heap->size &&
            node_less(heap->nodes[right], heap->nodes[smallest]))
        {
            smallest = right;
        }

        if (smallest == index)
            break;

        heap_swap(&heap->nodes[index], &heap->nodes[smallest]);
        index = smallest;
    }

    return result;
}

/* ==================== HUFFMAN TREE ==================== */

static HuffmanNode *build_tree(
    uint64_t frequencies[SYMBOLS],
    HuffmanNode nodes[MAX_NODES],
    int *node_count)
{
    MinHeap heap = { .size = 0 };

    *node_count = 0;

    for (int i = 0; i < SYMBOLS; i++)
    {
        if (frequencies[i] == 0)
            continue;

        HuffmanNode *node = &nodes[(*node_count)++];

        node->symbol = (unsigned char)i;
        node->frequency = frequencies[i];
        node->min_symbol = i;
        node->left = NULL;
        node->right = NULL;

        heap_push(&heap, node);
    }

    if (heap.size == 0)
        return NULL;

    while (heap.size > 1)
    {
        HuffmanNode *left = heap_pop(&heap);
        HuffmanNode *right = heap_pop(&heap);

        if (UINT64_MAX - left->frequency < right->frequency)
            return NULL;

        HuffmanNode *parent = &nodes[(*node_count)++];

        parent->symbol = 0;
        parent->frequency = left->frequency + right->frequency;

        parent->min_symbol =
            left->min_symbol < right->min_symbol
                ? left->min_symbol
                : right->min_symbol;

        parent->left = left;
        parent->right = right;

        heap_push(&heap, parent);
    }

    return heap_pop(&heap);
}

/* ==================== HUFFMAN CODES ==================== */

static void generate_codes(
    HuffmanNode *node,
    unsigned char current_code[MAX_CODE_LENGTH],
    int depth,
    unsigned char codes[SYMBOLS][MAX_CODE_LENGTH],
    unsigned short lengths[SYMBOLS])
{
    if (node == NULL)
        return;

    if (is_leaf(node))
    {
        if (depth == 0)
        {
            current_code[0] = 0;
            depth = 1;
        }

        memcpy(codes[node->symbol], current_code, (size_t)depth);
        lengths[node->symbol] = (unsigned short)depth;

        return;
    }

    if (depth >= MAX_CODE_LENGTH)
        return;

    current_code[depth] = 0;

    generate_codes(
        node->left,
        current_code,
        depth + 1,
        codes,
        lengths
    );

    current_code[depth] = 1;

    generate_codes(
        node->right,
        current_code,
        depth + 1,
        codes,
        lengths
    );
}

/* ==================== BIT WRITER ==================== */

typedef struct
{
    FILE *file;
    unsigned char buffer;
    int bit_count;
    int failed;

} BitWriter;

static void write_bit(BitWriter *writer, int bit)
{
    if (writer->failed)
        return;

    writer->buffer =
        (unsigned char)((writer->buffer << 1) | (bit & 1));

    writer->bit_count++;

    if (writer->bit_count == 8)
    {
        if (fputc(writer->buffer, writer->file) == EOF)
            writer->failed = 1;

        writer->buffer = 0;
        writer->bit_count = 0;
    }
}

static void flush_bits(BitWriter *writer)
{
    if (writer->failed || writer->bit_count == 0)
        return;

    writer->buffer =
        (unsigned char)(writer->buffer << (8 - writer->bit_count));

    if (fputc(writer->buffer, writer->file) == EOF)
        writer->failed = 1;

    writer->buffer = 0;
    writer->bit_count = 0;
}

/* ==================== BIT READER ==================== */

typedef struct
{
    FILE *file;
    unsigned char buffer;
    int bits_remaining;
    int failed;

} BitReader;

static int read_bit(BitReader *reader)
{
    if (reader->failed)
        return -1;

    if (reader->bits_remaining == 0)
    {
        int value = fgetc(reader->file);

        if (value == EOF)
        {
            reader->failed = 1;
            return -1;
        }

        reader->buffer = (unsigned char)value;
        reader->bits_remaining = 8;
    }

    int bit = (reader->buffer >> 7) & 1;

    reader->buffer <<= 1;
    reader->bits_remaining--;

    return bit;
}

/* ==================== FILE UTILITIES ==================== */

static int write_u64(FILE *file, uint64_t value)
{
    for (int i = 0; i < 8; i++)
    {
        if (fputc((int)(value & 0xFF), file) == EOF)
            return 0;

        value >>= 8;
    }

    return 1;
}

static int read_u64(FILE *file, uint64_t *value)
{
    uint64_t result = 0;

    for (int i = 0; i < 8; i++)
    {
        int byte = fgetc(file);

        if (byte == EOF)
            return 0;

        result |= (uint64_t)(unsigned char)byte << (i * 8);
    }

    *value = result;

    return 1;
}

static int get_file_size(const char *path, uint64_t *size)
{
    struct stat info;

    if (stat(path, &info) != 0 || info.st_size < 0)
        return 0;

    *size = (uint64_t)info.st_size;

    return 1;
}

static int same_file(const char *first, const char *second)
{
    struct stat a;
    struct stat b;

    if (strcmp(first, second) == 0)
        return 1;

    if (stat(first, &a) == 0 && stat(second, &b) == 0)
    {
        return a.st_dev == b.st_dev && a.st_ino == b.st_ino;
    }

    return 0;
}

/* ==================== COMPRESSION REPORT ==================== */

static void print_compression_report(
    uint64_t original_size,
    uint64_t compressed_size)
{
    double saved = 0.0;
    double ratio = 0.0;

    if (original_size > 0)
    {
        saved =
            (1.0 - (double)compressed_size / (double)original_size) * 100.0;

        ratio =
            ((double)compressed_size / (double)original_size) * 100.0;
    }

    printf("\n");
    printf("============================================\n");
    printf("             COMPRESSION REPORT             \n");
    printf("============================================\n");

    printf("Original file size   : %" PRIu64 " bytes\n", original_size);
    printf("Compressed file size : %" PRIu64 " bytes\n", compressed_size);

    if (compressed_size < original_size)
        printf("Space saved          : %.2f%%\n", saved);
    else if (compressed_size > original_size)
        printf("Space overhead       : %.2f%%\n", -saved);
    else
        printf("Space saved          : 0.00%%\n");

    printf("Compression ratio    : %.2f%%\n", ratio);

    printf("============================================\n\n");
}

/* ==================== COMPRESS FILE ==================== */

int huffman_compress_file(
    const char *input_path,
    const char *output_path)
{
    if (input_path == NULL || output_path == NULL ||
        input_path[0] == '\0' || output_path[0] == '\0')
    {
        printf("Invalid file path.\n");
        return 0;
    }

    if (same_file(input_path, output_path))
    {
        printf("Input and output must be different files.\n");
        return 0;
    }

    FILE *input = fopen(input_path, "rb");

    if (input == NULL)
    {
        printf("Cannot open input file: %s\n", input_path);
        return 0;
    }

    uint64_t frequencies[SYMBOLS] = {0};
    uint64_t original_size = 0;

    int byte;

    /* First pass: calculate byte frequencies. */

    while ((byte = fgetc(input)) != EOF)
    {
        if (frequencies[(unsigned char)byte] == UINT64_MAX ||
            original_size == UINT64_MAX)
        {
            printf("Input file is too large to process.\n");
            fclose(input);
            return 0;
        }

        frequencies[(unsigned char)byte]++;
        original_size++;
    }

    if (ferror(input))
    {
        printf("Error reading input file.\n");
        fclose(input);
        return 0;
    }

    rewind(input);

    HuffmanNode nodes[MAX_NODES];
    int node_count = 0;

    HuffmanNode *root = build_tree(
        frequencies,
        nodes,
        &node_count
    );

    if (original_size > 0 && root == NULL)
    {
        printf("Failed to build Huffman tree.\n");
        fclose(input);
        return 0;
    }

    unsigned char current_code[MAX_CODE_LENGTH] = {0};

    unsigned char codes[SYMBOLS][MAX_CODE_LENGTH] = {{0}};
    unsigned short lengths[SYMBOLS] = {0};

    generate_codes(
        root,
        current_code,
        0,
        codes,
        lengths
    );

    FILE *output = fopen(output_path, "wb");

    if (output == NULL)
    {
        printf("Cannot create output file: %s\n", output_path);
        fclose(input);
        return 0;
    }

    int success = 1;

    /* Write file signature. */

    if (fwrite(MAGIC, 1, sizeof(MAGIC), output) != sizeof(MAGIC))
        success = 0;

    /* Write original file size. */

    if (success && !write_u64(output, original_size))
        success = 0;

    /* Write frequency table. */

    for (int i = 0; i < SYMBOLS && success; i++)
    {
        if (!write_u64(output, frequencies[i]))
            success = 0;
    }

    /* Second pass: encode the original file. */

    if (success)
    {
        BitWriter writer = {
            .file = output,
            .buffer = 0,
            .bit_count = 0,
            .failed = 0
        };

        while ((byte = fgetc(input)) != EOF)
        {
            unsigned char symbol = (unsigned char)byte;

            for (int i = 0; i < lengths[symbol]; i++)
            {
                write_bit(&writer, codes[symbol][i]);

                if (writer.failed)
                    break;
            }

            if (writer.failed)
                break;
        }

        if (ferror(input) || writer.failed)
            success = 0;

        flush_bits(&writer);

        if (writer.failed)
            success = 0;
    }

    if (fclose(input) != 0)
        success = 0;

    if (fclose(output) != 0)
        success = 0;

    if (!success)
    {
        remove(output_path);
        printf("Compression failed due to a file I/O error.\n");
        return 0;
    }

    uint64_t compressed_size = 0;

    if (!get_file_size(output_path, &compressed_size))
    {
        printf("Compression completed, but output size could not be read.\n");
        return 0;
    }

    printf("\nFile compressed successfully!\n");

    print_compression_report(
        original_size,
        compressed_size
    );

    return 1;
}

/* ==================== DECOMPRESS FILE ==================== */

int huffman_decompress_file(
    const char *input_path,
    const char *output_path)
{
    if (input_path == NULL || output_path == NULL ||
        input_path[0] == '\0' || output_path[0] == '\0')
    {
        printf("Invalid file path.\n");
        return 0;
    }

    if (same_file(input_path, output_path))
    {
        printf("Input and output must be different files.\n");
        return 0;
    }

    FILE *input = fopen(input_path, "rb");

    if (input == NULL)
    {
        printf("Cannot open compressed file: %s\n", input_path);
        return 0;
    }

    unsigned char magic[8];

    if (fread(magic, 1, sizeof(magic), input) != sizeof(magic) ||
        memcmp(magic, MAGIC, sizeof(MAGIC)) != 0)
    {
        printf("Invalid MEOW compressed file.\n");
        fclose(input);
        return 0;
    }

    uint64_t original_size;

    if (!read_u64(input, &original_size))
    {
        printf("Corrupted compressed file.\n");
        fclose(input);
        return 0;
    }

    uint64_t frequencies[SYMBOLS];
    uint64_t frequency_sum = 0;

    for (int i = 0; i < SYMBOLS; i++)
    {
        if (!read_u64(input, &frequencies[i]))
        {
            printf("Corrupted frequency table.\n");
            fclose(input);
            return 0;
        }

        if (UINT64_MAX - frequency_sum < frequencies[i])
        {
            printf("Invalid frequency table.\n");
            fclose(input);
            return 0;
        }

        frequency_sum += frequencies[i];
    }

    if (frequency_sum != original_size)
    {
        printf("Compressed file contains an invalid size or frequency table.\n");
        fclose(input);
        return 0;
    }

    HuffmanNode nodes[MAX_NODES];
    int node_count = 0;

    HuffmanNode *root = build_tree(
        frequencies,
        nodes,
        &node_count
    );

    if (original_size > 0 && root == NULL)
    {
        printf("Cannot reconstruct Huffman tree.\n");
        fclose(input);
        return 0;
    }

    FILE *output = fopen(output_path, "wb");

    if (output == NULL)
    {
        printf("Cannot create output file: %s\n", output_path);
        fclose(input);
        return 0;
    }

    int success = 1;

    /* Empty file: nothing needs decoding. */

    if (original_size == 0)
    {
        if (root != NULL)
            success = 0;
    }
    else
    {
        BitReader reader = {
            .file = input,
            .buffer = 0,
            .bits_remaining = 0,
            .failed = 0
        };

        uint64_t decoded = 0;

        /* Special case: only one unique byte exists. */

        if (is_leaf(root))
        {
            for (uint64_t i = 0; i < original_size; i++)
            {
                if (fputc(root->symbol, output) == EOF)
                {
                    success = 0;
                    break;
                }
            }

            if (success && frequencies[root->symbol] != original_size)
                success = 0;
        }
        else
        {
            while (decoded < original_size)
            {
                HuffmanNode *current = root;

                while (!is_leaf(current))
                {
                    int bit = read_bit(&reader);

                    if (bit == -1)
                    {
                        success = 0;
                        break;
                    }

                    current = bit ? current->right : current->left;

                    if (current == NULL)
                    {
                        success = 0;
                        break;
                    }
                }

                if (!success)
                    break;

                if (fputc(current->symbol, output) == EOF)
                {
                    success = 0;
                    break;
                }

                decoded++;
            }
        }
    }

    if (fclose(input) != 0)
        success = 0;

    if (fclose(output) != 0)
        success = 0;

    if (!success)
    {
        remove(output_path);
        printf("Decompression failed. The file may be corrupted.\n");
        return 0;
    }

    printf("\nFile decompressed successfully!\n");
    printf("Restored file size: %" PRIu64 " bytes\n\n", original_size);

    return 1;
}
