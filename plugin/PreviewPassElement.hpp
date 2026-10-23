#pragma once
#include <hyprland/src/render/pass/PassElement.hpp>

// The translucent preview of the area a dragged window will take when the button is released.
class CPreviewPassElement : public IPassElement {
  public:
    struct SPreviewData {
        CBox       box;   // monitor-local, in physical pixels
        int        round = 0;
        int        border = 2;
        CHyprColor fill;
        CHyprColor line;
    };

    CPreviewPassElement(const SPreviewData& data_) : data(data_) {}
    virtual ~CPreviewPassElement() = default;

    virtual std::vector<UP<IPassElement>> draw() override;
    virtual bool                          needsLiveBlur() override {
        return false;
    }
    virtual bool needsPrecomputeBlur() override {
        return false;
    }
    virtual std::optional<CBox> boundingBox() override {
        return data.box;
    }
    virtual const char* passName() override {
        return "CPreviewPassElement";
    }
    virtual ePassElementType type() override {
        return EK_CUSTOM;
    }

  private:
    SPreviewData data;
};
