// SPDX-License-Identifier: BSD-3-Clause AND MIT
// Derived from hyprbars (hyprwm/hyprland-plugins, BSD-3-Clause, Copyright (c) 2023 Hypr Development); changes MIT. See NOTICE.
#pragma once
#include <hyprland/src/render/pass/PassElement.hpp>
#include <hyprland/src/render/WindowRenderPresentation.hpp>
#include "passLife.hpp"

class CHyprBar;

class CBarPassElement : public IPassElement {
  public:
    struct SBarData {
        CHyprBar*                         deco = nullptr;
        float                             a    = 1.F;
        Render::SWindowRenderPresentation presentation; // a copy: the host's reference ends with draw()
    };

    CBarPassElement(const SBarData& data_);
    virtual ~CBarPassElement() = default;

    virtual std::vector<UP<IPassElement>> draw(Render::CRenderContext& ctx) override;
    virtual bool                          needsLiveBlur(Render::CRenderContext& ctx) override;
    virtual bool                          needsPrecomputeBlur(Render::CRenderContext& ctx) override;
    virtual std::optional<CBox>           boundingBox(Render::CRenderContext& ctx) override;

    virtual const char*                   passName() override {
        return "CBarPassElement";
    }

    virtual ePassElementType type() override {
        return EK_CUSTOM;
    }

  private:
    SBarData      data;
    KwPass::SLife life;
};