#include <gtest/gtest.h>
#include <string>
#include <unicode/ubrk.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>

TEST(ICU_GraphemeTest, ClusterCount) {
  const char *utf8 =
      "A👨‍👩‍👧‍👦B"; // 3 grapheme clusters: A, family emoji, B
  UErrorCode status = U_ZERO_ERROR;
  UChar utf16[100];
  int32_t utf16_len = 0;
  u_strFromUTF8(utf16, 100, &utf16_len, utf8, -1, &status);
  ASSERT_TRUE(U_SUCCESS(status));

  UBreakIterator *bi =
      ubrk_open(UBRK_CHARACTER, "en_US", utf16, utf16_len, &status);
  ASSERT_TRUE(U_SUCCESS(status));
  int32_t start = ubrk_first(bi);
  int32_t end = ubrk_next(bi);
  int cluster_count = 0;
  while (end != UBRK_DONE) {
    cluster_count++;
    start = end;
    end = ubrk_next(bi);
  }
  ubrk_close(bi);
  // Should be 3 clusters: 'A', emoji, 'B'
  EXPECT_EQ(cluster_count, 3);
}

TEST(ICU_ConvertTest, ShiftJIS_UTF8_RoundTrip) {
  // "こんにちは" in Shift_JIS
  const unsigned char sjis_bytes[] = {0x82, 0xb1, 0x82, 0xf1, 0x82,
                                      0xc9, 0x82, 0xbf, 0x82, 0xcd};
  const int sjis_len = sizeof(sjis_bytes);
  UErrorCode status = U_ZERO_ERROR;

  // Convert Shift_JIS to UTF-8
  UConverter *conv_sjis = ucnv_open("Shift_JIS", &status);
  ASSERT_TRUE(U_SUCCESS(status));
  int32_t utf8_len = ucnv_toUChars(conv_sjis, nullptr, 0,
                                   (const char *)sjis_bytes, sjis_len, &status);
  status = U_ZERO_ERROR;
  std::vector<UChar> utf16(utf8_len);
  ucnv_toUChars(conv_sjis, utf16.data(), utf8_len, (const char *)sjis_bytes,
                sjis_len, &status);
  ASSERT_TRUE(U_SUCCESS(status));
  ucnv_close(conv_sjis);

  // Convert UTF-16 to UTF-8
  std::string utf8_str;
  int32_t utf8_bytes_len = 0;
  status = U_ZERO_ERROR;
  UConverter *conv_utf8 = ucnv_open("UTF-8", &status);
  ASSERT_TRUE(U_SUCCESS(status));
  utf8_bytes_len =
      ucnv_fromUChars(conv_utf8, nullptr, 0, utf16.data(), utf8_len, &status);
  status = U_ZERO_ERROR;
  utf8_str.resize(utf8_bytes_len);
  ucnv_fromUChars(conv_utf8, &utf8_str[0], utf8_bytes_len, utf16.data(),
                  utf8_len, &status);
  ASSERT_TRUE(U_SUCCESS(status));
  ucnv_close(conv_utf8);

  // Convert UTF-8 back to Shift_JIS
  status = U_ZERO_ERROR;
  UConverter *conv_sjis2 = ucnv_open("Shift_JIS", &status);
  ASSERT_TRUE(U_SUCCESS(status));
  int32_t sjis_out_len =
      ucnv_fromUChars(conv_sjis2, nullptr, 0, utf16.data(), utf8_len, &status);
  status = U_ZERO_ERROR;
  std::vector<char> sjis_out(sjis_out_len);
  ucnv_fromUChars(conv_sjis2, sjis_out.data(), sjis_out_len, utf16.data(),
                  utf8_len, &status);
  ASSERT_TRUE(U_SUCCESS(status));
  ucnv_close(conv_sjis2);

  // sjis_out should match original sjis_bytes
  EXPECT_EQ(std::vector<char>(sjis_bytes, sjis_bytes + sjis_len), sjis_out);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
