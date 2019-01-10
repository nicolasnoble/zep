#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "buffer.h"
#include "window_base.h"

namespace Zep
{

class ZepTabWindow;
class IZepDisplay;
struct Region;
struct Scroller;

// Line information, calculated during display update.
// A collection of spans that show split lines on the display
struct SpanInfo
{
    NVec2i columnOffsets;                 // Begin/end range of the text buffer for this line, as always end is one beyond the end.
    long lastNonCROffset = InvalidOffset; // The last char that is visible on the line (i.e. not CR/LF)
    float spanYPx = 0.0f;                 // Position in the buffer in pixels, if the screen was as big as the buffer.
    long bufferLineNumber = 0;            // Line in the original buffer, not the screen line
    int lineIndex = 0;

    long Length() const
    {
        return columnOffsets.y - columnOffsets.x;
    }
    bool BufferCursorInside(long offset) const
    {
        return offset >= columnOffsets.x && offset < columnOffsets.y;
    }
};

inline bool operator < (const SpanInfo& lhs, const SpanInfo& rhs)
{
    if (lhs.columnOffsets.x != rhs.columnOffsets.x)
    {
        return lhs.columnOffsets.x < rhs.columnOffsets.x;
    }
    return lhs.columnOffsets.y < rhs.columnOffsets.y;
}

enum class CursorType
{
    Hidden,
    Normal,
    Insert,
    Visual
};

enum class DisplayMode
{
    Normal,
    Vim
};

namespace WindowFlags
{
enum
{
    None = (0),
    ShowWhiteSpace = (1 << 0),
    ShowCR = (1 << 1)
};
}

struct AirBox
{
    std::string text;
    NVec4f background;
};

struct Airline
{
    std::vector<AirBox> leftBoxes;
    std::vector<AirBox> rightBoxes;
};


// Display state for a single pane of text.
// Window shows a buffer, and is parented by a TabWindow
// The buffer can change, but the window must always have an active buffer
// Editor operations such as select and change are local to a displayed pane
class ZepWindow : public ZepComponent
{
public:
    ZepWindow(ZepTabWindow& window, ZepBuffer* buffer);
    virtual ~ZepWindow();

    virtual void Notify(std::shared_ptr<ZepMessage> message) override;

    void SetCursorType(CursorType mode);
    void UpdateAirline();
    void UpdateScrollers();

    ZepTabWindow& GetTabWindow() const;

    void SetDisplayRegion(const NRectf& region);

    void Display();
    void DisplayCursor();

    void MoveCursorY(int yDistance, LineLocation clampLocation = LineLocation::LineLastNonCR);

    BufferLocation GetBufferCursor();
    void SetBufferCursor(BufferLocation location);

    // Flags
    void SetWindowFlags(uint32_t windowFlags);
    uint32_t GetWindowFlags() const;
    void ToggleFlag(uint32_t flag);

    long GetMaxDisplayLines();
    long GetNumDisplayedLines();

    ZepBuffer& GetBuffer() const;
    void SetBuffer(ZepBuffer* pBuffer);

    NVec2i BufferToDisplay();
    NVec2i BufferToDisplay(const BufferLocation& location);

    NVec2f GetTextSize(const utf8* pCh, const utf8* pEnd = nullptr);

    float ToWindowY(float pos) const;

    bool IsActiveWindow() const;
    NVec4f FilterActiveColor(const NVec4f& col);

private:
    struct WindowPass
    {
        enum Pass
        {
            Background = 0,
            Text,
            Max
        };
    };

private:
    void UpdateLayout();
    void UpdateLineSpans();
    void ScrollToCursor();
    void EnsureCursorVisible();
    void UpdateVisibleLineRange();
    bool IsInsideTextRegion(NVec2i pos) const;

    const SpanInfo& GetCursorLineInfo(long y);

    bool DisplayLine(const SpanInfo& lineInfo, const NRectf& region, int displayPass);
    void DisplayScrollers();

private:
    NVec2f ToBufferRegion(const NVec2f& pos);
    std::shared_ptr<Region> m_bufferRegion;  // region of the display we are showing on.
    std::shared_ptr<Region> m_textRegion;    // region of the display for text.
    std::shared_ptr<Region> m_airlineRegion; // Airline
    std::shared_ptr<Region> m_numberRegion;     // Numbers
    std::shared_ptr<Region> m_indicatorRegion;  // Indicators 
    std::shared_ptr<Region> m_vScrollRegion;    // Vertical scroller

    bool m_wrap = true;     // Wrap

    // The buffer offset is where we are looking, but the cursor is only what you see on the screen
    CursorType m_cursorType = CursorType::Normal; // Type of cursor
    DisplayMode m_displayMode = DisplayMode::Vim; // Vim editing mode

    // Visual stuff
    std::vector<std::string> m_statusLines; // Status information, shown under the buffer

    float m_bufferOffsetYPx = 0.0f;
    float m_bufferSizeYPx = 0.0f;
    NVec2i m_visibleLineRange = {0, 0};  // Offset of the displayed area into the text

    std::vector<SpanInfo*> m_windowLines; // Information about the currently displayed lines

    ZepTabWindow& m_tabWindow;

    uint32_t m_windowFlags = WindowFlags::ShowWhiteSpace;

    long m_maxDisplayLines = 0;
    float m_defaultLineSize = 0;

    BufferLocation m_bufferCursor{0}; // Location in buffer coordinates.  Each window has a different buffer cursor
    long m_lastCursorColumn = 0;      // The last cursor column (could be removed and recalculated)

    ZepBuffer* m_pBuffer = nullptr;
    Airline m_airline;
    std::unordered_map<Zep::utf8, NVec2f> m_mapCharSizes;

    bool m_layoutDirty = true;
    bool m_scrollVisibilityChanged = true;
    bool m_cursorMoved = true;

    std::shared_ptr<Scroller> m_vScroller;
};

} // namespace Zep
