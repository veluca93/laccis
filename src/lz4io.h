#ifndef LZ4IO_HPP
#define LZ4IO_HPP
#include <lz4frame.h>
#include <stdio.h>


#define LZ4_FOOTER_SIZE 4
static const unsigned buf_size = 1024*1024;

static const LZ4F_preferences_t lz4_preferences = {
	{ LZ4F_max256KB, LZ4F_blockLinked, LZ4F_noContentChecksum, LZ4F_frame, 0, { 0, 0 } },
	7,   /* compression level */
	0,   /* autoflush */
	{ 0, 0, 0, 0 },  /* reserved, must be set to 0 */
};

class lz4io {
private:
    unsigned char dst[buf_size];
    FILE* fout;
    LZ4F_compressionContext_t ctx;
    unsigned dst_off;
public:
    lz4io(FILE* fout): fout(fout) {
	    LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
	    dst_off = LZ4F_compressBegin(ctx, dst, buf_size, &lz4_preferences);
    }

    void append(const std::string& s) {
        if (dst_off+s.size() > buf_size/2) {
            fwrite(dst, 1, dst_off, fout);
            dst_off = 0;
        }
        dst_off += LZ4F_compressUpdate(ctx, dst+dst_off, buf_size-dst_off, s.c_str(), s.size(), NULL);
    }

    ~lz4io() {
        fwrite(dst, 1, dst_off, fout);
        dst_off = LZ4F_compressEnd(ctx, dst, buf_size, NULL);
        fwrite(dst, 1, dst_off, fout);
	    if (ctx) LZ4F_freeCompressionContext(ctx);
    }
};

#endif
