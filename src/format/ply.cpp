#include "voxelio/format/ply.hpp"

#include "voxelio/macro.hpp"

namespace voxelio::ply {

ResultCode Writer::init() noexcept
{
    if (initialized) {
        return ResultCode::WARNING_DOUBLE_INIT;
    }
    initialized = true;

    // Note: our header is always exactly 300 bytes long.
    // When removing the header bytes, the format is 100% bit-identical to VL32
    // This is because we fill the in the vertex count (which is the only variable length in the header) in the
    // ....;....; etc. area.
    stream.writeString("ply\r\n");
    stream.writeString("format binary_big_endian 1.0\r\n");
    stream.writeString("comment generated by voxel-io: a C++ library by Jan \"Eisenwave\" Schultke\r\n");
    stream.writeString("element vertex ");  // we will fill this in upon destruction
    this->vertexCountOffset = stream.position();
    stream.writeString("....;....;....;....;....;...\r\n");
    stream.writeString("property int x\r\n");
    stream.writeString("property int y\r\n");
    stream.writeString("property int z\r\n");
    stream.writeString("property uchar alpha\r\n");
    stream.writeString("property uchar red\r\n");
    stream.writeString("property uchar green\r\n");
    stream.writeString("property uchar blue\r\n");
    stream.writeString("end_header\r\n");

    return stream.good() ? ResultCode::OK : ResultCode::WRITE_ERROR_IO_FAIL;
}

ResultCode Writer::write(Voxel32 buffer[], size_t bufferLength) noexcept
{
    if (not initialized) {
        VXIO_FORWARD_ERROR(init());
    }
    VXIO_DEBUG_ASSERT(initialized);

    for (size_t i = 0; i < bufferLength; ++i) {
        VXIO_FORWARD_ERROR(writeVoxel(buffer[i]));
    }
    return ResultCode::WRITE_OK;
}

ResultCode Writer::writeVoxel(Voxel32 v) noexcept
{
    ++voxelCount;
    stream.writeBig<v.pos.size, i32>(v.pos.data());
    stream.writeBig<argb32>(v.argb);
    return stream.good() ? ResultCode::OK : ResultCode::WRITE_ERROR_IO_FAIL;
}

Writer::~Writer()
{
    // TODO do this in a flush() function instead of a destructor

    stream.seekAbsolute(vertexCountOffset);

    std::string str = stringifyDec(voxelCount) + "\r\ncomment ";
    stream.writeString(str);

    VXIO_ASSERT(stream.good());
}

}  // namespace voxelio::ply
