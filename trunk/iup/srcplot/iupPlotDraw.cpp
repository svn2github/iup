
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "iupPlot.h"
#include "iupcbs.h"


static inline void iPlotSetLine(cdCanvas* canvas, int inLineStyle, int inLineWidth)
{
  cdCanvasLineStyle(canvas, inLineStyle);
  cdCanvasLineWidth(canvas, inLineWidth);
}

static inline void iPlotSetMark(cdCanvas* canvas, int inMarkStyle, int inMarkSize)
{
  cdCanvasMarkType(canvas, inMarkStyle);
  cdCanvasMarkSize(canvas, inMarkSize);
}

static inline void iPlotDrawText(cdCanvas* canvas, double inX, double inY, int inAlignment, const char* inString)
{
  cdCanvasTextAlignment(canvas, inAlignment);
  cdfCanvasText(canvas, inX, inY, inString);
}

static inline void iPlotDrawRotatedText(cdCanvas* canvas, double inX, double inY, double inDegrees, int inAlignment, const char *inString)
{
  cdCanvasTextAlignment(canvas, inAlignment);
  double theOldOrientation = cdCanvasTextOrientation(canvas, inDegrees);
  cdfCanvasText(canvas, inX, inY, inString);
  cdCanvasTextOrientation(canvas, theOldOrientation);
}

static inline void iPlotDrawRect(cdCanvas* canvas, int inX, int inY, int inW, int inH)
{
  cdCanvasRect(canvas, inX, inX + inW - 1, inY, inY + inH - 1);
}

static inline void iPlotDrawBox(cdCanvas* canvas, double inX, double inY, double inW, double inH)
{
  cdfCanvasBox(canvas, inX, inX + inW - 1, inY, inY + inH - 1);
}

static void iPlotFillArrow(cdCanvas* canvas, int inX1, int inY1, int inX2, int inY2, int inX3, int inY3)
{
  cdCanvasBegin(canvas, CD_FILL);
  cdCanvasVertex(canvas, inX1, inY1);
  cdCanvasVertex(canvas, inX2, inY2);
  cdCanvasVertex(canvas, inX3, inY3);
  cdCanvasEnd(canvas);
}

static void iPlotDrawArrow(cdCanvas* canvas, double inX, double inY, int inVertical, int inDirection, int inSize)
{
  int theX = iupPlotRound(inX);
  int theY = iupPlotRound(inY);

  inSize += 2; // to avoid too small sizes

  int theSizeDir = iupPlotRound(inSize * 0.7);
  if (inVertical)
  {
    cdfCanvasLine(canvas, inX, inY, inX, inY + inDirection*inSize);  // complement the axis

    int theY1 = iupPlotRound(inY + inDirection*inSize);
    int theY2 = theY1 - inDirection*theSizeDir;
    iPlotFillArrow(canvas, theX, theY1,
                   theX - theSizeDir, theY2,
                   theX + theSizeDir, theY2);
  }
  else
  {
    cdfCanvasLine(canvas, inX, inY, inX + inDirection*inSize, inY);

    int theX1 = iupPlotRound(inX + inDirection*inSize);
    int theX2 = theX1 - inDirection*theSizeDir;
    iPlotFillArrow(canvas, theX1, theY,
                   theX2, theY - theSizeDir,
                   theX2, theY + theSizeDir);
  }
}


/************************************************************************************/


void iupPlotBox::Draw(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  cdCanvasSetForeground(canvas, mColor);
  iPlotSetLine(canvas, mLineStyle, mLineWidth);

  iPlotDrawRect(canvas, inRect.mX, inRect.mY, inRect.mWidth, inRect.mHeight);
}

bool iupPlotGrid::DrawX(iupPlotTickIterBase* inTickIter, iupPlotTrafoBase* inTrafo, const iupPlotRect &inRect, cdCanvas* canvas) const
{
  if (mShowX) 
  {
    if (!inTickIter->Init())
      return false;

    double theX;
    bool theIsMajorTick;

    cdCanvasSetForeground(canvas, mColor);
    iPlotSetLine(canvas, mLineStyle, mLineWidth);

    while (inTickIter->GetNextTick(theX, theIsMajorTick, NULL))
    {          
      if (theIsMajorTick) 
      {
        double theScreenX = inTrafo->Transform(theX);
        cdfCanvasLine(canvas, theScreenX, inRect.mY, theScreenX, inRect.mY + inRect.mHeight-1);
      }
    }
  }

  return true;
}

bool iupPlotGrid::DrawY(iupPlotTickIterBase* inTickIter, iupPlotTrafoBase* inTrafo, const iupPlotRect &inRect, cdCanvas* canvas) const
{
  if (mShowY)
  {
    if (!inTickIter->Init())
      return false;

    double theY;
    bool theIsMajorTick;
    iupPlotRect theTickRect;

    cdCanvasSetForeground(canvas, mColor);
    iPlotSetLine(canvas, mLineStyle, mLineWidth);

    while (inTickIter->GetNextTick(theY, theIsMajorTick, NULL))
    {            
      if (theIsMajorTick) 
      {
        double theScreenY = inTrafo->Transform(theY);
        cdfCanvasLine(canvas, inRect.mX, theScreenY, inRect.mX + inRect.mWidth - 1, theScreenY);
      }
    }
  }

  return true;
}

bool iupPlotAxis::DrawX(const iupPlotRect &inRect, cdCanvas* canvas, const iupPlotAxis& inAxisY) const
{
  if (!mShow)
    return true;

  cdCanvasSetForeground(canvas, mColor);
  iPlotSetLine(canvas, CD_CONTINUOUS, mLineWidth);

  double theTargetY = 0;
  if (!mCrossOrigin) 
  {
    if (!inAxisY.mReverse) 
      theTargetY = inAxisY.mMin;
    else 
      theTargetY = inAxisY.mMax;
  }

  double theScreenY  = inAxisY.mTrafo->Transform(theTargetY);
  double theScreenX1 = inRect.mX;
  double theScreenX2 = theScreenX1 + inRect.mWidth;

  cdfCanvasLine(canvas, theScreenX1, theScreenY, theScreenX2, theScreenY);

  if (mShowArrow)
  {
    if (!mReverse)
      iPlotDrawArrow(canvas, theScreenX2, theScreenY, 0, 1, mTick.mMinorSize);
    else
      iPlotDrawArrow(canvas, theScreenX1, theScreenY, 0, -1, mTick.mMinorSize);
  }

  if (mTick.mShow) 
  {
    if (!mTickIter->Init())
      return false;

    double theX;
    bool theIsMajorTick;
    char theFormatString[30];
    strcpy(theFormatString, mTick.mFormatString);

    if (mTick.mShowNumber)
      SetFont(canvas, mTick.mFontStyle, mTick.mFontSize);

    while (mTickIter->GetNextTick(theX, theIsMajorTick, theFormatString))
    {
      if (!DrawXTick(theX, theScreenY, theIsMajorTick, theFormatString, canvas))
        return false;
    }

    theScreenY -= mTick.mMajorSize;  // skip major tick
    if (mTick.mShowNumber)
    {
      if (mTick.mRotateNumber)
      {
        int theXTickNumberWidth;
        GetTickNumberSize(canvas, &theXTickNumberWidth, NULL);
        theScreenY -= theXTickNumberWidth;
      }
      else
      {
        int theXTickNumberHeight;
        GetTickNumberSize(canvas, NULL, &theXTickNumberHeight);
        theScreenY -= theXTickNumberHeight;
      }
    }
  }

  if (GetLabel()) 
  {
    int theXFontHeight;
    SetFont(canvas, mFontStyle, mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theXFontHeight, NULL, NULL);

    theScreenY -= theXFontHeight / 10;  // spacing

    if (mLabelCentered)
    {
      double theScreenX = theScreenX1 + inRect.mWidth / 2;
      iPlotDrawText(canvas, theScreenX, theScreenY, CD_NORTH, GetLabel());
    }
    else
    {
      double theScreenX = theScreenX2;
      iPlotDrawText(canvas, theScreenX, theScreenY, CD_NORTH_EAST, GetLabel());
    }
  }

  return true;
}

bool iupPlotAxis::DrawXTick(double inX, double inScreenY, bool inMajor, const char*inFormatString, cdCanvas* canvas) const
{
  int theTickSize;
  double theScreenX = mTrafo->Transform(inX);
  if (inMajor) 
  {
    theTickSize = mTick.mMajorSize;

    if (mTick.mShowNumber)
    {
      char theBuf[128];
      sprintf(theBuf, inFormatString, inX);
      double theScreenY = inScreenY - theTickSize - mTick.mMinorSize;  // Use minor size as spacing
      if (mTick.mRotateNumber)
        iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_EAST, theBuf);
      else
        iPlotDrawText(canvas, theScreenX, theScreenY, CD_NORTH, theBuf);
    }
  }
  else 
    theTickSize = mTick.mMinorSize;

  cdfCanvasLine(canvas, theScreenX, inScreenY, theScreenX, inScreenY - theTickSize);
  return true;
}

bool iupPlotAxis::DrawY(const iupPlotRect &inRect, cdCanvas* canvas, const iupPlotAxis& inAxisX) const
{
  if (!mShow)
    return true;

  cdCanvasSetForeground(canvas, mColor);
  iPlotSetLine(canvas, CD_CONTINUOUS, mLineWidth);

  double theTargetX = 0;
  if (!mCrossOrigin) 
  {
    if (!inAxisX.mReverse) 
      theTargetX = inAxisX.mMin;
    else
      theTargetX = inAxisX.mMax;
  }
  if (inAxisX.mDiscrete)
    theTargetX -= 0.5;

  double theScreenX  = inAxisX.mTrafo->Transform(theTargetX);
  double theScreenY1 = inRect.mY;
  double theScreenY2 = theScreenY1 + inRect.mHeight;

  cdfCanvasLine(canvas, theScreenX, theScreenY1, theScreenX, theScreenY2);

  if (mShowArrow)
  {
    if (!mReverse)
      iPlotDrawArrow(canvas, theScreenX, theScreenY2, 1, 1, mTick.mMinorSize);
    else
      iPlotDrawArrow(canvas, theScreenX, theScreenY1, 1, -1, mTick.mMinorSize);
  }

  if (mTick.mShow) 
  {
    if (!mTickIter->Init())
      return false;

    double theY;
    bool theIsMajorTick;
    char theFormatString[30];
    strcpy(theFormatString, mTick.mFormatString);

    if (mTick.mShowNumber)
      SetFont(canvas, mTick.mFontStyle, mTick.mFontSize);

    while (mTickIter->GetNextTick(theY, theIsMajorTick, theFormatString))
    {
      if (!DrawYTick(theY, theScreenX, theIsMajorTick, theFormatString, canvas))
        return false;
    }

    theScreenX -= mTick.mMajorSize;  // skip major tick
    if (mTick.mShowNumber)
    {
      if (mTick.mRotateNumber)
      {
        int theYTickNumberHeight;
        GetTickNumberSize(canvas, NULL, &theYTickNumberHeight);
        theScreenX -= theYTickNumberHeight;
      }
      else
      {
        int theYTickNumberWidth;
        GetTickNumberSize(canvas, &theYTickNumberWidth, NULL);
        theScreenX -= theYTickNumberWidth;
      }
    }
  }

  if (GetLabel()) 
  {
    int theYFontHeight;
    SetFont(canvas, mFontStyle, mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theYFontHeight, NULL, NULL);

    theScreenX -= theYFontHeight / 10;  // spacing

    if (mLabelCentered)
    {
      double theScreenY = theScreenY1 + inRect.mHeight / 2;
      iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_SOUTH, GetLabel());
    }
    else
    {
      double theScreenY = theScreenY2;
      iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_SOUTH_EAST, GetLabel());
    }
  }

  return true;
}

bool iupPlotAxis::DrawYTick(double inY, double inScreenX, bool inMajor, const char* inFormatString, cdCanvas* canvas) const
{
  int theTickSize;
  double theScreenY = mTrafo->Transform(inY);
  if (inMajor) 
  {
    theTickSize = mTick.mMajorSize;

    if (mTick.mShowNumber)
    {
      char theBuf[128];
      sprintf(theBuf, inFormatString, inY);
      double theScreenX = inScreenX - theTickSize - mTick.mMinorSize;  // Use minor size as spacing
      if (mTick.mRotateNumber)
        iPlotDrawRotatedText(canvas, theScreenX, theScreenY, 90, CD_SOUTH, theBuf);
      else
        iPlotDrawText(canvas, theScreenX, theScreenY, CD_EAST, theBuf);
    }
  }
  else 
    theTickSize = mTick.mMinorSize;

  cdfCanvasLine(canvas, inScreenX, theScreenY, inScreenX - theTickSize, theScreenY);  
  return true;
}

bool iupPlot::GetCrossPoint(const iupPlotDataBase *inXData, const iupPlotDataBase *inYData, int &outY) const
{
  int theCount = inXData->GetCount();
  if (theCount == 0)
    return false;
  
  double theXTarget = mAxisX.mTrafo->TransformBack((double)mCrossHairX);
  bool theFirstIsLess = inXData->GetValue(0) < theXTarget;

  for (int i = 0; i < theCount; i++)
  {
    double theX = inXData->GetValue(i);
    bool theCurrentIsLess = theX < theXTarget;

    if (theCurrentIsLess != theFirstIsLess) 
    {
      double theY = inYData->GetValue(i);
      outY = iupPlotRound(mAxisY.mTrafo->Transform(theY)); // transform to pixels
      return true;
    }
  }

  return false;
}

void iupPlot::DrawCrossHair(const iupPlotRect &inRect, cdCanvas* canvas) const
{
  cdCanvasSetForeground(canvas, mAxisY.mColor);
  iPlotSetLine(canvas, CD_CONTINUOUS, 1);

  // Draw a vertical line at cursor X coordinate
  cdCanvasLine(canvas, mCrossHairX, inRect.mY, mCrossHairX, inRect.mY + inRect.mHeight - 1);

  for (int ds = 0; ds < mDataSetListCount; ds++)
  {
    iupPlotDataSet* dataset = mDataSetList[ds];

    iupPlotDataBase *theXData = dataset->mDataX;
    iupPlotDataBase *theYData = dataset->mDataY;

    int theScreenY;
    if (GetCrossPoint(theXData, theYData, theScreenY))
    {
      cdCanvasSetForeground(canvas, dataset->mColor);

      // Draw a horizontal line at data Y coordinate
      cdfCanvasLine(canvas, inRect.mX, theScreenY, inRect.mX + inRect.mWidth - 1, theScreenY);
    }
  }
}

void iupPlot::SetTitleFont(cdCanvas* canvas) const
{
  int theFontSize = mTitle.mFontSize;
  if (theFontSize == 0)
  {
    int size = IupGetInt(ih, "FONTSIZE");
    if (size > 0) size += 6;
    else size -= 8;

    theFontSize = size;
  }

  SetFont(canvas, mTitle.mFontStyle, theFontSize);
}

void iupPlot::DrawPlotTitle(cdCanvas* canvas) const
{
  if (mTitle.GetText()) 
  {
    cdCanvasSetForeground(canvas, mTitle.mColor);

    SetTitleFont(canvas);

    // do not depend on theMargin
    int theX = mViewport.mWidth / 2;
    int theY = mViewport.mHeight - 5;  // add small spacing

    cdCanvasTextAlignment(canvas, CD_NORTH);
    cdCanvasText(canvas, theX, theY, mTitle.GetText());
  }
}

void iupPlot::DrawPlotBackground(cdCanvas* canvas) const
{
  cdCanvasSetForeground(canvas, mBackColor);
  cdCanvasBox(canvas, 0, mViewport.mWidth - 1, 0, mViewport.mHeight - 1);
}

bool iupPlot::DrawLegend (const iupPlotRect &inRect, cdCanvas* canvas) const 
{
  if (mLegend.mShow)
  {
    int ds;
    int theFontHeight;

    SetFont(canvas, mLegend.mFontStyle, mLegend.mFontSize);
    cdCanvasGetFontDim(canvas, NULL, &theFontHeight, NULL, NULL);

    int theMargin = theFontHeight / 2;
    if (mLegend.mPosition == IUP_PLOT_BOTTOMCENTER)
      theMargin = 0;
    int theTotalHeight = mDataSetListCount*theFontHeight + 2 * theMargin;
    int theLineSpace = 20;

    int theWidth, theMaxWidth = 0;
    for (ds = 0; ds < mDataSetListCount; ds++)
    {
      iupPlotDataSet* dataset = mDataSetList[ds];

      cdCanvasGetTextSize(canvas, dataset->GetName(), &theWidth, NULL);

      if (dataset->mMode == IUP_PLOT_MARK || dataset->mMode == IUP_PLOT_MARKLINE)
      {
        if (dataset->mMarkSize + 6 > theLineSpace)
          theLineSpace = dataset->mMarkSize + 6;
      }

      theWidth += theLineSpace;

      if (theWidth > theMaxWidth)
        theMaxWidth = theWidth;
    }

    if (theMaxWidth == 0)
      return false;

    theMaxWidth += 2 * theMargin;

    int theScreenX = inRect.mX;
    int theScreenY = inRect.mY;

    switch (mLegend.mPosition)
    {
    case IUP_PLOT_TOPLEFT:
      theScreenX += 2;
      theScreenY += inRect.mHeight - theTotalHeight - 2;
      break;
    case IUP_PLOT_BOTTOMLEFT:
      theScreenX += 2;
      theScreenY += 2;
      break;
    case IUP_PLOT_BOTTOMRIGHT:
      theScreenX += inRect.mWidth - theMaxWidth - 2;
      theScreenY += 2;
      break;
    case IUP_PLOT_BOTTOMCENTER:
      theScreenX += (inRect.mWidth - theMaxWidth) / 2;
      theScreenY = theFontHeight / 4;
      break;
    default: // IUP_PLOT_TOPRIGHT
      theScreenX += inRect.mWidth - theMaxWidth - 2;
      theScreenY += inRect.mHeight - theTotalHeight - 2;
      break;
    }

    cdCanvasClipArea(canvas, theScreenX, theScreenX + theMaxWidth - 1, 
                             theScreenY, theScreenY + theTotalHeight - 1);

    cdCanvasSetForeground(canvas, mLegend.mBoxBackColor);
    iPlotDrawBox(canvas, theScreenX+1, theScreenY+1, theMaxWidth-2, theTotalHeight-2);

    cdCanvasSetForeground(canvas, mLegend.mBoxColor);
    iPlotSetLine(canvas, mLegend.mBoxLineStyle, mLegend.mBoxLineWidth);
    iPlotDrawRect(canvas, theScreenX, theScreenY, theMaxWidth, theTotalHeight);

    for (ds = 0; ds < mDataSetListCount; ds++)
    {
      iupPlotDataSet* dataset = mDataSetList[ds];

      cdCanvasSetForeground(canvas, dataset->mColor);

      int theLegendX = theScreenX + theMargin;
      int theLegendY = theScreenY + (mDataSetListCount-1 - ds)*theFontHeight + theMargin;

      theLegendY += theFontHeight / 2;

      if (dataset->mMode == IUP_PLOT_MARK || dataset->mMode == IUP_PLOT_MARKLINE)
      {
        iPlotSetMark(canvas, dataset->mMarkStyle, dataset->mMarkSize);
        cdCanvasMark(canvas, theLegendX + (theLineSpace - 3) / 2, theLegendY - theFontHeight / 8);
      }
      if (dataset->mMode != IUP_PLOT_MARK)
      {
        iPlotSetLine(canvas, dataset->mLineStyle, dataset->mLineWidth);
        cdCanvasLine(canvas, theLegendX, theLegendY - theFontHeight / 8,
                             theLegendX + theLineSpace - 3, theLegendY - theFontHeight / 8);
      }

      iPlotDrawText(canvas, theLegendX + theLineSpace, theLegendY, CD_WEST, dataset->GetName());
    }
  }

  return true;
}


/************************************************************************************/


void iupPlotDataSet::DrawDataLine(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas) const
{
  int theCount = mDataX->GetCount();
  cdCanvasBegin(canvas, CD_OPEN_LINES);

  for (int i = 0; i < theCount; i++) 
  {
    double theX = mDataX->GetValue(i);
    double theY = mDataY->GetValue(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    cdfCanvasVertex(canvas, theScreenX, theScreenY);
  }

  cdCanvasEnd(canvas);
}

void iupPlotDataSet::DrawDataMark(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas) const
{
  int theCount = mDataX->GetCount();
  for (int i = 0; i < theCount; i++)
  {
    double theX = mDataX->GetValue(i);
    double theY = mDataY->GetValue(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    cdCanvasMark(canvas, iupPlotRound(theScreenX), iupPlotRound(theScreenY));
  }
}

void iupPlotDataSet::DrawDataMarkLine(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas) const
{
  int theCount = mDataX->GetCount();
  cdCanvasBegin(canvas, CD_OPEN_LINES);

  for (int i = 0; i < theCount; i++) 
  {
    double theX = mDataX->GetValue(i);
    double theY = mDataY->GetValue(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    cdCanvasMark(canvas, iupPlotRound(theScreenX), iupPlotRound(theScreenY));
    cdfCanvasVertex(canvas, theScreenX, theScreenY);
  }

  cdCanvasEnd(canvas);
}

void iupPlotDataSet::DrawDataArea(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas) const
{
  int theCount = mDataX->GetCount();
  cdCanvasBegin(canvas, CD_FILL);

  for (int i = 0; i < theCount; i++) 
  {
    double theX = mDataX->GetValue(i);
    double theY = mDataY->GetValue(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    if (i == 0) 
    {
      double theScreenY0 = inTrafoY->Transform(0);
      cdfCanvasVertex(canvas, theScreenX, theScreenY0);
    }

    cdfCanvasVertex(canvas, theScreenX, theScreenY);

    if (i == theCount-1)
    {
      double theScreenY0 = inTrafoY->Transform(0);
      cdfCanvasVertex(canvas, theScreenX, theScreenY0);
    }
  }

  cdCanvasEnd(canvas);
}

void iupPlotDataSet::DrawDataBar(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas) const
{
  int theCount = mDataX->GetCount();
  double theScreenY0 = inTrafoY->Transform(0);

  double theMinX = mDataX->GetValue(0);
  double theScreenMinX = inTrafoX->Transform(theMinX);
  double theMaxX = mDataX->GetValue(theCount-1);
  double theScreenMaxX = inTrafoX->Transform(theMaxX);

  double theBarWidth = (theScreenMaxX - theScreenMinX + 1) / theCount;
  theBarWidth *= 0.9;

  for (int i = 0; i < theCount; i++) 
  {
    double theX = mDataX->GetValue(i);
    double theY = mDataY->GetValue(i);
    double theScreenX = inTrafoX->Transform(theX);
    double theScreenY = inTrafoY->Transform(theY);

    double theBarX = theScreenX - theBarWidth / 2;
    double theBarHeight = theScreenY - theScreenY0;

    iPlotDrawBox(canvas, theBarX, theScreenY0, theBarWidth, theBarHeight);
  }
}

void iupPlotDataSet::DrawData(const iupPlotTrafoBase *inTrafoX, const iupPlotTrafoBase *inTrafoY, cdCanvas* canvas) const
{
  int theXCount = mDataX->GetCount();
  int theYCount = mDataY->GetCount();

  if ((theXCount == 0) || (theYCount == 0))
    return;

  if (theXCount != theYCount)
    return;

  cdCanvasSetForeground(canvas, mColor);
  iPlotSetLine(canvas, mLineStyle, mLineWidth);
  iPlotSetMark(canvas, mMarkStyle, mMarkSize);

  switch (mMode)
  {
  case IUP_PLOT_LINE:
    DrawDataLine(inTrafoX, inTrafoY, canvas);
    break;
  case IUP_PLOT_MARK:
    DrawDataMark(inTrafoX, inTrafoY, canvas);
    break;
  case IUP_PLOT_MARKLINE:
    DrawDataMarkLine(inTrafoX, inTrafoY, canvas);
    break;
  case IUP_PLOT_AREA:
    DrawDataArea(inTrafoX, inTrafoY, canvas);
    break;
  case IUP_PLOT_BAR:
    DrawDataBar(inTrafoX, inTrafoY, canvas);
    break;
  }
}

