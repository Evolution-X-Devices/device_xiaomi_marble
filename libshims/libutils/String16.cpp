#include <utils/String16.h>

namespace android {

static const StaticString16 emptyString(u"");
static inline char16_t* getEmptyString() {
    return const_cast<char16_t*>(emptyString.c_str());
}

// ---------------------------------------------------------------------------

String16::String16(String16&& o) noexcept
    : mString(o.mString)
{
    o.mString = getEmptyString();
}

String16& String16::operator=(String16&& other) noexcept {
    release();
    mString = other.mString;
    other.mString = getEmptyString();
    return *this;
}

}; // namespace android
