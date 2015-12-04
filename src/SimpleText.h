#if 0

#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include <unordered_map>
#include "GLB.h"

class SimpleText
{
public:
	enum Alignment {
		LEFT = 0,
		BOTTOM = 0,
		CENTER = 1,
		RIGHT = 2,
		TOP = 2
	};

	struct StringMetrics {
		int left, right;
		int top, bottom;
		int width, height;

		vec2i PixelSize() const		{return v2i(width, height);}

		int AlignLeftX() const		{return -left;}
		int AlignCenterX() const	{return (-right - left) / 2;}
		int AlignRightX() const		{return -right;}

		int AlignTopY() const		{return -top;}
		int AlignCenterY() const	{return (-top + bottom) / 2;}
		int AlignBottomY() const	{return bottom;}

		vec2i AlignLeft() const		{return v2i(AlignLeftX(), AlignBottomY());}
		vec2i AlignCenter() const	{return v2i(AlignCenterX(), AlignBottomY());}
		vec2i AlignRight() const	{return v2i(AlignRightX(), AlignBottomY());}
		
		vec2i Align(Alignment alignment) const {
			switch (alignment) {
				default:
				case LEFT: return AlignLeft();
				case CENTER: return AlignCenter();
				case RIGHT: return AlignRight();
			}
		}
		vec2i Align(Alignment halignment, Alignment valignment) const {
			int x, y;
			switch (halignment) {
				default:
				case LEFT: x = AlignLeftX(); break;
				case CENTER: x = AlignCenterX(); break;
				case RIGHT: x = AlignRightX(); break;
			}
			switch (valignment) {
				default:
				case TOP: y = AlignTopY(); break;
				case CENTER: y = AlignCenterY(); break;
				case BOTTOM: y = AlignBottomY(); break;
			}
			return v2i(x, y);
		}
	};
public:
	static void Init();
	static void Destroy();
	static void LoadFont(std::string filename);
	static bool UnloadFont(std::string filename);

public:
	SimpleText();
	SimpleText(std::string filename, unsigned int dpi, float pointSize, vec2i resolution);
	virtual ~SimpleText();

	unsigned int ExchangeDPI(unsigned int dpi);
	float ExchangeSize(float pointSize);
	vec4f ExchangeColor(vec4f color);
	vec2i ExchangeResolution(vec2i res);
	Alignment ExchangeHorizontalAlignment(Alignment align);
	Alignment ExchangeVerticalAlignment(Alignment align);
	std::string ExchangeFont(std::string filename, bool unloadCurrentFont = false);

	bonobo::Texture *ToTexture(StringMetrics &metrics, std::string str);
	bonobo::Texture *ToTexture(StringMetrics &metrics, std::wstring str);

	void Print(vec2i pixelPos, std::string str);
	void Print(vec2i pixelPos, std::wstring str);
	void Print(vec2f ndcPos, std::string str);
	void Print(vec2f ndcPos, std::wstring str);

private:
	static bool sInitiated;
	static FT_Library sFt;
	static std::unordered_map<std::string, FT_Face> sFontMap;
	unsigned int mDpi;
	float mSize;
	vec4f mColor;
	vec2i mResolution;
	Alignment mHAlignment;
	Alignment mVAlignment;
	int mLineWidth;
	std::string mFont;
};

extern SimpleText gSimpleText;

#endif
