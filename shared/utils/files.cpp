#include "files.h"

std::unordered_map<unsigned long long, quesync::file_chunk> quesync::utils::files::get_file_chunks(
    std::ifstream &file_stream, unsigned long long file_size) {
    std::unordered_map<unsigned long long, quesync::file_chunk> chunks;
    unsigned long long amount_of_chunks;
    file_chunk chunk;

    // Calculate amount of chunks in file
    amount_of_chunks = utils::files::calc_amount_of_chunks(file_size);

    // For each chunk in the file
    for (unsigned long long i = 0; i < amount_of_chunks; i++) {
        // Allocate the chunk
        chunk.data = std::shared_ptr<unsigned char>(new unsigned char[FILE_CHUNK_SIZE]);

        // Set the chunk's index
        chunk.index = i;

        // Get a chunk from the file
        file_stream.read((char *)chunk.data.get(), FILE_CHUNK_SIZE);

        // Set the chunk
        chunks[i] = chunk;
    }

    return chunks;
}

unsigned long long quesync::utils::files::calc_amount_of_chunks(unsigned long long file_size) {
    return file_size / FILE_CHUNK_SIZE + (file_size % FILE_CHUNK_SIZE ? 1 : 0);
}