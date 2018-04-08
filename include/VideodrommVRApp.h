#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/AxisAlignedBox.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/Sphere.h"
#include "cinder/Timeline.h"

#include "cinder/vr/vr.h"
// UserInterface
#include "CinderImGui.h"
// Warping
#include "Warp.h"
// Settings
#include "VDSettings.h"
// Session
#include "VDSession.h"
// UI
#include "VDUI.h"

using namespace ci;
using namespace ci::app;
using namespace ph::warping;
using namespace VideoDromm;

#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))


class UiIntersectable;
using UiIntersectablePtr = std::shared_ptr<UiIntersectable>;

//! \class UiIntersectable
//!
//!
class UiIntersectable {
public:

	virtual bool intersects(const ci::Ray &ray) const = 0;
	virtual void draw() = 0;

	bool getFocused() const {
		return mFocused;
	}

	void setFocused(bool value) {
		if (value != mFocused) {
			mFocused = value;
			float target = (mFocused ? 1.0f : 0.0f);
			ci::app::App::get()->timeline().apply(&mFocusedValue, target, 0.25f);
		}
	}

	bool getSelected() const {
		return mSelected;
	}

	virtual void setSelected(bool value) {
		if (value != mSelected) {
			mSelected = value;
			float target = (mSelected ? 1.0f : 0.0f);
			ci::app::App::get()->timeline().apply(&mSelectedValue, target, 0.25f);
		}
	}

	ColorA getColor() const {
		float t = mSelectedValue.value();
		Color color = (1.0f - t)*Color(0.4f, 0.4f, 0.42f) + t*Color(0.95f, 0.5f, 0.0f);
		t = mFocusedValue.value();
		color = (1.0f - t)*color + t*Color(0.9f, 0.9f, 0.0f);
		return ColorA(color);
	}

	const ci::vec3& getPosition() const {
		return mPosition;
	}

protected:
	gl::GlslProgRef		mShader;
	gl::BatchRef		mBatch;
	bool				mFocused = false;
	ci::Anim<float>		mFocusedValue = 0.0f;
	bool				mSelected = false;
	ci::Anim<float>		mSelectedValue = 0.0f;
	ci::vec3			mPosition;
};

//! \class UiBox
//!
//!
class UiBox : public UiIntersectable {
public:
	UiBox() {}

	UiBox(const vec3 &center, const vec3 &size, const gl::GlslProgRef &shader) {
		vec3 min = center - 0.5f*size;
		vec3 max = center + 0.5f*size;
		mBox = AxisAlignedBox(min, max);
		mShader = shader;
		mBatch = gl::Batch::create(geom::Cube().size(size) >> geom::Translate(center), shader);
		mPosition = center;
	}

	virtual bool intersects(const ci::Ray &ray) const override {
		bool result = false;
		float t0 = std::numeric_limits<float>::min();
		float t1 = std::numeric_limits<float>::min();
		if (mBox.intersect(ray, &t0, &t1) && (t0 > 0.0f)) {
			result = true;
		}
		return result;
	}

	virtual void draw() {
		gl::ScopedDepth scopedDepth(false);

		gl::ScopedBlendAlpha scopeBlend;
		gl::ScopedColor scopedColor(getColor());
		mBatch->draw();
	}
protected:
	AxisAlignedBox mBox;
};




class VideodrommVRApp : public App {

public:
	void setup() override;
	void cleanup() override;
	void update();
	void draw() override;

	void fileDrop(FileDropEvent event) override;
	void resize() override;

	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;

	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;

	void setUIVisibility(bool visible);
private:
	// Settings
	VDSettingsRef				mVDSettings;
	// Session
	VDSessionRef				mVDSession;

	// UI
	VDUIRef						mVDUI;

	// warping
	gl::TextureRef				mImage;
	WarpList					mWarps;
	string						fileWarpsName;
	fs::path					mWarpSettings;
	// fbo
	gl::FboRef					mRenderFbo;
	unsigned int				mWarpFboIndex;
	bool						mFadeInDelay;
	// shaders
	gl::GlslProgRef				mShader;
	// Cinder VR
	ci::vr::Context				*mVrContext = nullptr;
	ci::vr::Hmd					*mHmd = nullptr;
	bool						mCyclopsMirroring = false;
	CameraPersp					mCamera;

	bool						mRecalcOrigin = false;

	const ci::vr::Controller	*mController1 = nullptr;
	const ci::vr::Controller	*mController2 = nullptr;

	gl::BatchRef				mGridBox;
	gl::BatchRef				mGridLines;

	std::vector<UiIntersectablePtr>	mShapes;
	uint32_t						mShapeIndex = 0;

	void	onControllerConnect(const ci::vr::Controller *controller);
	void	onControllerDisconnect(const ci::vr::Controller *controller);
	void	onButtonDown(const ci::vr::Controller::Button *button);
	void	onButtonUp(const ci::vr::Controller::Button *button);

	void	initGrid(const gl::GlslProgRef &shader);
	void	initShapes(const gl::GlslProgRef &shader);

	void	drawScene();
};