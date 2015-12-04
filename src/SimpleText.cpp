#if 0

#include <unordered_map>
#include <vector>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "VectorMath.h"
#include "Log.h"
#include "GLB.h"
#include "lodepng.h"
#include "SDL_timer.h"
#include "SimpleText.h"
#include "SimpleDraw.h"

#define STR_MAX_LEN			4096

SimpleText gSimpleText;

bool SimpleText::sInitiated = false;
FT_Library SimpleText::sFt;
std::unordered_map<std::string, FT_Face> SimpleText::sFontMap;

SimpleText::SimpleText()
{
	mDpi = 70;
	mSize = 18.0f;
	mColor = v4f(1.0f, 1.0f, 1.0f, 1.0f);
	mResolution = v2i(256, 256);
	mHAlignment = SimpleText::LEFT;
	mVAlignment = SimpleText::BOTTOM;
	mFont = "";
}

SimpleText::SimpleText(std::string filename, unsigned int dpi, float pointSize, vec2i resolution)
{
	mColor = v4f(1.0f, 1.0f, 1.0f, 1.0f);
	mHAlignment = SimpleText::LEFT;
	mVAlignment = SimpleText::BOTTOM;

	ExchangeDPI(dpi);
	ExchangeSize(pointSize);
	ExchangeResolution(resolution);
	ExchangeFont(filename);
}

SimpleText::~SimpleText()
{
}

void SimpleText::Init()
{
	if (sInitiated)
		return;
	if (FT_Init_FreeType(&sFt)) {
		LogWarning("Could not initiate FreeType. Text rendering will not work\n");
		return;
	} else
		sInitiated = true;
}

void SimpleText::Destroy()
{
	if (FT_Done_FreeType(sFt))
		LogWarning("Error closing FreeType.\n");
	sInitiated = false;
}

void SimpleText::LoadFont(std::string filename)
{
	if (!sInitiated)
		Init();
	auto elem = sFontMap.find(filename);
	if (elem != sFontMap.end())
		return;
	FT_Face face;
	std::string fn = "res/" + filename;
	if (FT_New_Face(sFt, fn.c_str(), 0, &face)) {
		LogWarning("Could not open font %s\n", fn.c_str());
		return;
	}
	sFontMap[filename] = face;
}

bool SimpleText::UnloadFont(std::string filename)
{
	if (!sInitiated)
		Init();
	auto elem = sFontMap.find(filename);
	if (elem == sFontMap.end())
		return false;
	FT_Done_Face(elem->second);
	sFontMap.erase(filename);
	return true;
}

unsigned int SimpleText::ExchangeDPI(unsigned int dpi)
{
	unsigned int old = mDpi;
	mDpi = dpi;
	return old;
}

float SimpleText::ExchangeSize(float size)
{
	float old = mSize;
	mSize = size;
	return old;
}

vec4f SimpleText::ExchangeColor(vec4f color)
{
	vec4f old = mColor;
	mColor = color;
	return old;
}

vec2i SimpleText::ExchangeResolution(vec2i res)
{
	vec2i old = mResolution;
	mResolution = res;
	return old;
}

SimpleText::Alignment SimpleText::ExchangeHorizontalAlignment(SimpleText::Alignment align)
{
	Alignment old = mHAlignment;
	mHAlignment = align;
	return old;
}

SimpleText::Alignment SimpleText::ExchangeVerticalAlignment(SimpleText::Alignment align)
{
	Alignment old = mVAlignment;
	mVAlignment = align;
	return old;
}

std::string SimpleText::ExchangeFont(std::string filename, bool unloadCurrentFont)
{
	if (unloadCurrentFont && !mFont.empty())
		UnloadFont(mFont);
	LoadFont(filename);
	std::string old = mFont;
	mFont = filename;
	return old;
}

bonobo::Texture *SimpleText::ToTexture(StringMetrics &metrics, std::string str)
{
	std::wstring wstr;
	wstr.assign(str.begin(), str.end());
	return ToTexture(metrics, wstr);
}

bonobo::Texture *SimpleText::ToTexture(StringMetrics &metrics, std::wstring str)
{
	if (!sInitiated)
		Init();

	if (mFont.empty()) {
		LogWarning("No font set\n");
		return nullptr;
	}
	auto elem = sFontMap.find(mFont);
	if (elem == sFontMap.end()) {
		LogWarning("Font is not loaded\n");
		return nullptr;
	}
	FT_Face face = elem->second;

	int size = int(mSize * 64.0f + 0.5f);
	FT_Error error = FT_Set_Char_Size(face, size, 0, mDpi, 0);
	FT_GlyphSlot slot = face->glyph;

	FT_Vector pen;

	/* the pen position in 26.6 cartesian space coordinates; */
	/* start at (300,200) relative to the upper left corner  */
	pen.x = 0;
	pen.y = 0;

	int left = INT_MAX, bottom = INT_MAX;
	int right = -INT_MAX, top = -INT_MAX;

	// Determine size
	for (size_t n = 0; n < str.size(); n++) {
		error = FT_Load_Char(face, str[n], FT_LOAD_RENDER);
		if (error)
			continue;
		left = smin<int>(left, (pen.x >> 6) + slot->bitmap_left);
		right = smax<int>(right, ((pen.x + 1) >> 6) + slot->bitmap_left + slot->bitmap.width);
		top = smax<int>(top, (pen.y >> 6) + slot->bitmap_top);
		bottom = smin<int>(bottom, ((pen.y + 1) >> 6) + slot->bitmap_top - slot->bitmap.rows);
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}
	int w = smax(2 + (right - left), 64);
	int h = smax(2 + (top - bottom), 64);
	int oh = 2 + (top - bottom);

	w = (((w - 1) >> 4) + 1) << 4;
	h = (((h - 1) >> 4) + 1) << 4;

	// TODO: Offset
	//pen.x = (left + 1) << 6;
	//pen.y = (bottom + 1) << 6;
	pen.x = 0;
	pen.y = 0;
	
	vec3uc color = cast<u8>(mColor.xyz() * 255.0f);
	u8 *img = new u8[w * h * 4];
	for (int i = 0; i < w * h; i++) {
		img[i * 4 + 0] = color.r;
		img[i * 4 + 1] = color.g;
		img[i * 4 + 2] = color.b;
		img[i * 4 + 3] = 0x00;
	}
	for (size_t n = 0; n < str.size(); n++) {
		error = FT_Load_Char(face, str[n], FT_LOAD_RENDER);
		if (error)
			continue;
		int ox = (pen.x >> 6) + left + 1;
		int oy = (pen.y >> 6) + top + 1 - slot->bitmap_top;

		for (int y = 0; y < slot->bitmap.rows; y++) {
			for (int x = 0; x < slot->bitmap.width; x++) {
				int X = x + ox;
				int Y = oh - (y + oy) - 1;
				if (X < 0 || X >= w || Y < 0 || Y >= h)
					continue;
				img[(X + Y * w) * 4 + 3] = u8(float(slot->bitmap.buffer[x + y * slot->bitmap.width]) * mColor.a);
			}
		}
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}

	bonobo::Texture *tex = bonobo::loadTexture2D(img, w, h, TEXTURE_UNORM, v4i(8, 8, 8, 8));

	delete [] img;

	//pixelSize = vec2i(w, h);
	//pixelOffset = vec2i(-left, bottom);
	metrics.width = w;
	metrics.height = h;
	metrics.bottom = bottom;
	metrics.top = top;
	metrics.left = left;
	metrics.right = right;
	//pixelOffset = vec2i(-left, -(h - top) - bottom);

	return tex;
}

void SimpleText::Print(vec2i pixelPos, std::string str)
{
	std::wstring wstr;
	wstr.assign(str.begin(), str.end());
	Print(pixelPos, wstr);
}

void SimpleText::Print(vec2i pixelPos, std::wstring str)
{
	if (!sInitiated)
		Init();
	StringMetrics metrics;
	bonobo::Texture *tex = ToTexture(metrics, str);
	vec2i pixelOffset = metrics.Align(mHAlignment, mVAlignment) + pixelPos;
	//vec2i pixelOffset = metrics.Align(mAlignment);
	vec2i pixelSize = metrics.PixelSize();

	vec2f scale = 2.0f * cast<f32>(pixelSize) / cast<float>(mResolution);
	vec2f offset = -1.0f + 2.0f * cast<f32>(pixelOffset) / cast<float>(mResolution);

	gSimpleDraw.Init();

	mat4f m = gSimpleDraw.ExchangeMatrix(mat4f::Identity());
	gSimpleDraw.Texture(offset, offset + scale, *tex, v4i(0, 1, 2, 3));
	gSimpleDraw.ExchangeMatrix(m);

	bonobo::unloadTexture(tex); // TODO: Use caching to avoid this
}

void SimpleText::Print(vec2f ndcPos, std::string str)
{
	vec2i pixelPos = cast<int>(cast<float>(mResolution) * (0.5f + 0.5f * ndcPos) + 0.5f);
	return Print(pixelPos, str);
}

void SimpleText::Print(vec2f ndcPos, std::wstring str)
{
	vec2i pixelPos = cast<int>(cast<float>(mResolution) * (0.5f + 0.5f * ndcPos) + 0.5f);
	return Print(pixelPos, str);
}

#endif
