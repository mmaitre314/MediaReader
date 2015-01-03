#pragma once

inline WSS::IBuffer^ CreateBuffer(_In_reads_(size) const unsigned char *src, _In_ size_t size)
{
    auto buffer = ref new WSS::Buffer((unsigned int)size);
    buffer->Length = buffer->Capacity;

    unsigned char *dst = GetData(buffer);
    std::copy(src, src + size, dst);

    return buffer;
}

class HttpMultipartMessage
{
public:

    void HttpMultipartMessage::SetBoundary(_In_ Platform::String^ boundary)
    {
        _boundary.resize(boundary->Length());
        size_t n;
        wcstombs_s(&n, &_boundary[0], _boundary.capacity(), boundary->Data(), boundary->Length());

        // RFC1341 7.2 says double hyphens in the part delimiter are not part of the Content-Type boundary header
        // parameter but some cameras add them anyway
        // http://www.w3.org/Protocols/rfc1341/7_2_Multipart.html
        if (_boundary == "--myboundary")
        {
            _boundary = "myboundary";
        }
    }

    void HttpMultipartMessage::Append(_In_ WSS::IBuffer^ buffer)
    {
        unsigned char* data = GetData(buffer);
        _buffer.insert(_buffer.end(), data, data + buffer->Length);
    }

    // Returns one part in the multipart message, or nullptr if not enough data received yet.
    _Ret_maybenull_ WSS::IBuffer^ HttpMultipartMessage::GetPart()
    {
        std::string markerBegin("\r\n\r\n");
        std::string markerEnd("\r\n--" + _boundary + "\r\n");

        auto pos0 = std::search(_buffer.begin(), _buffer.end(), markerBegin.begin(), markerBegin.end());
        if (pos0 == _buffer.end())
        {
            return nullptr;
        }

        auto pos1 = std::search(pos0 + 2, _buffer.end(), markerEnd.begin(), markerEnd.end());
        if (pos1 == _buffer.end())
        {
            return nullptr;
        }

        WSS::IBuffer^ buffer = CreateBuffer(&pos0[0] + 4, pos1 - (pos0 + 4));
        _buffer.erase(_buffer.begin(), pos1);

        return buffer;
    }

    void HttpMultipartMessage::Clear()
    {
        _buffer.clear();
    }

private:

    std::string _boundary;
    std::vector<unsigned char> _buffer;

};

