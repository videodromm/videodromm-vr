#include "VideodrommVRApp.h"

void VideodrommVRApp::initGrid(const gl::GlslProgRef &shader)
{
	vec3 size = vec3(100, 20, 100);

	// Box
	{
		mGridBox = gl::Batch::create(geom::Cube().size(1.01f*size) >> geom::Translate(0.0f, 0.0f, 0.0f), shader);
	}

	// Lines
	{
		Rectf bounds = Rectf(-size.x / 2.0f, -size.z / 2.0f, size.x / 2.0f, size.z / 2.0f);
		int nx = static_cast<int>(size.x);
		int nz = static_cast<int>(size.z);
		float dx = bounds.getWidth() / static_cast<float>(nx);
		float dz = bounds.getHeight() / static_cast<float>(nz);
		float y = 0.01f;

		std::vector<vec3> vertices;

		for (int i = 0; i <= nx; ++i) {
			float x = bounds.x1 + i * dx;
			vec3 p0 = vec3(x, y, bounds.y1);
			vec3 p1 = vec3(x, y, bounds.y2);
			vertices.push_back(p0);
			vertices.push_back(p1);
		}

		for (int i = 0; i <= nz; ++i) {
			float z = bounds.y1 + i * dz;
			vec3 p0 = vec3(bounds.x1, y, z);
			vec3 p1 = vec3(bounds.x2, y, z);
			vertices.push_back(p0);
			vertices.push_back(p1);
		}

		gl::VboRef vbo = gl::Vbo::create(GL_ARRAY_BUFFER, vertices);
		geom::BufferLayout layout = geom::BufferLayout({ geom::AttribInfo(geom::Attrib::POSITION, 3, sizeof(vec3), 0) });
		gl::VboMeshRef vboMesh = gl::VboMesh::create(static_cast<uint32_t>(vertices.size()), GL_LINES, { std::make_pair(layout, vbo) });
		mGridLines = gl::Batch::create(vboMesh, shader);
	}
}

void VideodrommVRApp::initShapes(const gl::GlslProgRef &shader)
{
	mShapes.clear();
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(0, -0.0625f, 3), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(-4, -0.0625f, -4), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(4, -0.0625f, -4), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(0, -0.0625f, -10), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(-4, -0.0625f, -4) + vec3(0, 5, -12), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(4, -0.0625f, -4) + vec3(0, 8, -12), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(-8, -0.0625f, 3), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(8, -0.0625f, 3), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(-16, -0.0625f, 3) + vec3(0, 4, 0), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(16, -0.0625f, 3) + vec3(0, 4, 0), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(-16, -0.0625f, 3) + vec3(0, 6.0f, -9), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(16, -0.0625f, 3) + vec3(0, 2.5f, -9), vec3(4, 0.125f, 4), shader)));

	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(-4, -0.0625f, 4) + vec3(0, 8, 12), vec3(4, 0.125f, 4), shader)));
	mShapes.push_back(UiIntersectablePtr(new UiBox(vec3(0, -0.001f, 0) + vec3(4, -0.0625f, 4) + vec3(0, 5, 12), vec3(4, 0.125f, 4), shader)));
}


void VideodrommVRApp::setup()
{
	// Settings
	mVDSettings = VDSettings::create();
	// Session
	mVDSession = VDSession::create(mVDSettings);
	// Utils
	mVDUtils = VDUtils::create(mVDSettings);
	// Animation
	mVDAnimation = VDAnimation::create(mVDSettings, mVDSession);
	// Message router
	mVDRouter = VDRouter::create(mVDSettings, mVDAnimation, mVDSession);
	// Mix

	mMixesFilepath = getAssetPath("") / mVDSettings->mAssetsPath / "mixes.xml";
	if (fs::exists(mMixesFilepath)) {
		// load textures from file if one exists
		mMixes = VDMix::readSettings(mVDSettings, mVDAnimation, mVDRouter, loadFile(mMixesFilepath));
	}
	else {
		// otherwise create a texture from scratch
		mMixes.push_back(VDMix::create(mVDSettings, mVDAnimation, mVDRouter));
	}
	mMixes[0]->setLeftFboIndex(2);
	mMixes[0]->setRightFboIndex(1);
	mVDAnimation->tapTempo();
	// UI
	mVDUI = VDUI::create(mVDSettings, mMixes[0], mVDRouter, mVDAnimation, mVDSession);
	// UI fbo
	gl::Fbo::Format format;
	//mUIFbo = gl::Fbo::create(1000, 800, format.colorTexture());
	setFrameRate(mVDSession->getTargetFps());
	// maximize fps
	disableFrameRate();

	// Cinder VR
	mCamera.lookAt(vec3(0, 0, 3), vec3(0, 0, 0));

	gl::disableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::color(Color::white());

	try {
		ci::vr::initialize();
	}
	catch (const std::exception& e) {
		CI_LOG_E("VR failed: " << e.what());
	}

	try {
		mVrContext = ci::vr::beginSession(
			ci::vr::SessionOptions()
			.setTrackingOrigin(ci::vr::TRACKING_ORIGIN_SEATED)
			.setOriginOffset(vec3(0, -1, -3))
			.setControllersScanInterval(0.25f)
			.setControllerConnected(std::bind(&VideodrommVRApp::onControllerConnect, this, std::placeholders::_1))
			.setControllerDisconnected(std::bind(&VideodrommVRApp::onControllerDisconnect, this, std::placeholders::_1))
			);
	}
	catch (const std::exception& e) {
		CI_LOG_E("Session failed: " << e.what());
	}

	try {
		mHmd = mVrContext->getHmd();
	}
	catch (const std::exception& e) {
		CI_LOG_E("Hmd failed: " << e.what());
	}

	mVrContext->getSignalControllerButtonDown().connect(std::bind(&VideodrommVRApp::onButtonDown, this, std::placeholders::_1));
	mVrContext->getSignalControllerButtonUp().connect(std::bind(&VideodrommVRApp::onButtonUp, this, std::placeholders::_1));

	auto shader = gl::getStockShader(gl::ShaderDef().color());
	mShader = gl::GlslProg::create(mMixes[0]->getVertexShaderString(0), mMixes[0]->getFragmentShaderString(0));

	initGrid(shader);
	initShapes(mShader);
}
void VideodrommVRApp::keyUp(KeyEvent event)
{
	// pass this key event to the warp editor first
	if (!Warp::handleKeyUp(mWarps, event)) {
		// let your application perform its keyUp handling here
		if (!mVDAnimation->handleKeyUp(event)) {
			// Animation did not handle the key, so handle it here
		}
	}
}
void VideodrommVRApp::keyDown(KeyEvent event)
{
	int i = 0;
	switch (event.getChar()) {
	case '1': {
		mCyclopsMirroring = false;
		if (mHmd) {
			mHmd->setMirrorMode(ci::vr::Hmd::MirrorMode::MIRROR_MODE_STEREO);
		}
	}
			  break;

	case '2': {
		mCyclopsMirroring = false;
		if (mHmd) {
			mHmd->setMirrorMode(ci::vr::Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_STEREO);
		}
	}
			  break;

	case '3': {
		mCyclopsMirroring = false;
		if (mHmd) {
			mHmd->setMirrorMode(ci::vr::Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_MONO_LEFT);
		}
	}
			  break;

	case '4': {
		mCyclopsMirroring = false;
		if (mHmd) {
			mHmd->setMirrorMode(ci::vr::Hmd::MirrorMode::MIRROR_MODE_UNDISTORTED_MONO_RIGHT);
		}
	}
			  break;

	case '5': {
		mCyclopsMirroring = true;
		if (mHmd) {
			mHmd->setMirrorMode(ci::vr::Hmd::MirrorMode::MIRROR_MODE_NONE);
		}
	}
			  break;
	case KeyEvent::KEY_ESCAPE:
		// quit the application
		quit();
		break;

	case KeyEvent::KEY_h:
		// mouse cursor
		mVDSettings->mCursorVisible = !mVDSettings->mCursorVisible;
		setUIVisibility(mVDSettings->mCursorVisible);
		break;
	case KeyEvent::KEY_x:
		i = 1;
		break;
	case KeyEvent::KEY_c:
		i = 2;
		break;
	case KeyEvent::KEY_v:
		i = 3;
		break;
	case KeyEvent::KEY_b:
		i = 4;
		break;
	case KeyEvent::KEY_n:
		i = 5;
		break;
	case KeyEvent::KEY_w:
		i = 6;
		break;
	}
	if (i > 0) {
		mShader = gl::GlslProg::create(mMixes[0]->getVertexShaderString(0), mMixes[0]->getFragmentShaderString(i));
		initShapes(mShader);

	}
}
void VideodrommVRApp::fileDrop(FileDropEvent event)
{
	int index = (int)(event.getX() / (mVDSettings->uiElementWidth + mVDSettings->uiMargin));// +1;
	//ci::fs::exists(path)
	//ci::fs::path
	ci::fs::path mPath = event.getFile(event.getNumFiles() - 1);
	string mFile = mPath.string();
	if (mMixes[0]->loadFileFromAbsolutePath(mFile, index) > -1) {
		// load success
		// reset zoom
		mVDAnimation->controlValues[22] = 1.0f;
	}
}
void VideodrommVRApp::setUIVisibility(bool visible)
{
	if (visible)
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
}
void VideodrommVRApp::onControllerConnect(const ci::vr::Controller* controller)
{
	if (!controller) {
		return;
	}

	bool connected = false;
	if (ci::vr::API_OCULUS == controller->getApi()) {
	}
	else if (ci::vr::API_OPENVR == controller->getApi()) {
		switch (controller->getType()) {
		case ci::vr::Controller::TYPE_LEFT: {
			mController1 = controller;
			connected = true;
		}
											break;

		case ci::vr::Controller::TYPE_RIGHT: {
			mController2 = controller;
			connected = true;
		}
											 break;
		}
	}

	if (connected) {
		CI_LOG_I("Controller connected: " << controller->getName());
	}
}

void VideodrommVRApp::onControllerDisconnect(const ci::vr::Controller* controller)
{
	if (!controller) {
		return;
	}

	bool disconnected = false;
	if (ci::vr::API_OCULUS == controller->getApi()) {
	}
	else if (ci::vr::API_OPENVR == controller->getApi()) {
		switch (controller->getType()) {
		case ci::vr::Controller::TYPE_LEFT: {
			mController1 = nullptr;
			disconnected = true;
		}
											break;

		case ci::vr::Controller::TYPE_RIGHT: {
			mController2 = nullptr;
			disconnected = true;
		}
											 break;
		}
	}

	if (disconnected) {
		CI_LOG_I("Controller disconnected: " << controller->getName());
	}
}

void VideodrommVRApp::onButtonDown(const ci::vr::Controller::Button *button)
{
	uint32_t shapdeIndex = mShapeIndex;
	for (uint32_t i = 0; i < mShapes.size(); ++i) {
		auto& shape = mShapes[i];
		if (shape->intersects(mHmd->getInputRay())) {
			shapdeIndex = i;
			break;
		}
	}

	if (shapdeIndex != mShapeIndex) {
		mShapeIndex = shapdeIndex;
		vec3 position = mShapes[mShapeIndex]->getPosition();
		mHmd->setLookAt(position);
	}
}

void VideodrommVRApp::onButtonUp(const ci::vr::Controller::Button *button)
{
}

void VideodrommVRApp::update()
{
	mVDSettings->iFps = getAverageFps();
	mVDSettings->sFps = toString(floor(mVDSettings->iFps));
	mVDAnimation->update();
	mVDRouter->update();
	mMixes[0]->update();
	// check if a shader has been received from websockets
	if (mVDSettings->mShaderToLoad != "") {
		mMixes[0]->loadFboFragmentShader(mVDSettings->mShaderToLoad, 1);
	}
	mShader->uniform("iGlobalTime", mVDSettings->iGlobalTime);
	//mShader->uniform("iResolution", vec3(mVDSettings->mPreviewFboWidth, mVDSettings->mPreviewFboHeight, 1.0));
	mShader->uniform("iResolution", vec3(4, 0.125f, 3));
	mShader->uniform("iChannelResolution", mVDSettings->iChannelResolution, 4);
	mShader->uniform("iMouse", vec4(mVDSettings->mRenderPosXY.x, mVDSettings->mRenderPosXY.y, mVDSettings->iMouse.z, mVDSettings->iMouse.z));//iMouse =  Vec3i( event.getX(), mRenderHeight - event.getY(), 1 );
	mShader->uniform("iChannel0", 0);
	mShader->uniform("iChannel1", 1);
	mShader->uniform("iChannel2", 2);
	for (auto s : mShapes)
	{
		if (s->getSelected()) mVDSettings->mMsg = "Selected " + toString(s->getPosition());
	}
	// Vive sometimes returns the wrong pose data initially so reinitialize the origin matrix after the first 60 frames.
	if ((ci::vr::API_OPENVR == mVrContext->getApi()) && (!mRecalcOrigin) && (mHmd->getElapsedFrames() > 60)) {
		mHmd->calculateOriginMatrix();
		mRecalcOrigin = true;
	}

	for (auto& shape : mShapes) {
		bool focused = shape->intersects(mHmd->getInputRay());
		shape->setFocused(focused);
	}
}
void VideodrommVRApp::cleanup()
{
	CI_LOG_V("shutdown");
	ui::Shutdown();
	// save warp settings
	Warp::writeSettings(mWarps, writeFile(mWarpSettings));
	mVDSettings->save();
	mVDSession->save();
	quit();
}
void VideodrommVRApp::resize() {
	CI_LOG_V("resizeWindow");
	mVDUI->resize();

	// tell the warps our window has been resized, so they properly scale up or down
	Warp::handleResize(mWarps);

	mVDSettings->iResolution.x = mVDSettings->mRenderWidth;
	mVDSettings->iResolution.y = mVDSettings->mRenderHeight;
}
void VideodrommVRApp::mouseMove(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseMove(mWarps, event)) {
		// let your application perform its mouseMove handling here
	}
}

void VideodrommVRApp::mouseDown(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseDown(mWarps, event)) {
		// let your application perform its mouseDown handling here
		mVDAnimation->controlValues[21] = event.getX() / getWindowWidth();
	}
}

void VideodrommVRApp::mouseDrag(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseDrag(mWarps, event)) {
		// let your application perform its mouseDrag handling here
	}
}
void VideodrommVRApp::mouseUp(MouseEvent event)
{
	// pass this mouse event to the warp editor first
	if (!Warp::handleMouseUp(mWarps, event)) {
		// let your application perform its mouseUp handling here
	}
}
void VideodrommVRApp::drawScene()
{
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Draw ground grid
	{
		gl::ScopedLineWidth scopedLineWidth(1.0f);

		gl::color(0.8f, 0.8f, 0.8f);
		mGridBox->draw();

		gl::color(0.65f, 0.65f, 0.65f);
		mGridLines->draw();
	}

	// Draw shape
	for (auto& shape : mShapes) {
		shape->draw();
	}

	// Draw coordinate frame
	{
		gl::drawCoordinateFrame(1.0f);
	}

	// Draw controller input rays
	{
		gl::lineWidth(3.0f);

		float s = 1000.0f;

		if (mController1 && mController1->hasInputRay()) {
			const auto& ir = mController1->getInputRay();
			vec3 dir = ir.getDirection();
			vec3 orig = ir.getOrigin() + (0.055f * dir);
			gl::color(0.9f, 0.5f, 0.0f);
			gl::drawLine(orig, orig + (s * dir));
		}

		if (mController2 && mController2->hasInputRay()) {
			const auto& ir = mController2->getInputRay();
			vec3 dir = ir.getDirection();
			vec3 orig = ir.getOrigin() + (0.055f * dir);
			gl::color(0.9f, 0.5f, 0.0f);
			gl::drawLine(orig, orig + (s * dir));
		}
	}

	// Draw HMD input ray (cyan sphere)
	{
		const auto& ir = mHmd->getInputRay();
		vec3 dir = ir.getDirection();
		vec3 P = ir.getOrigin() + (5.0f * dir);
		gl::color(0.01f, 0.7f, 0.9f);
		gl::drawSphere(P, 0.04f);
	}
}

void VideodrommVRApp::draw()
{
	gl::clear(Color::black());

	if (mHmd) {
		mHmd->bind();
		for (auto eye : mHmd->getEyes()) {
			mHmd->enableEye(eye);
			drawScene();
			mHmd->drawControllers(eye);
		}
		mHmd->unbind();

		// Draw mirrored
		if (mCyclopsMirroring) {
			mHmd->submitFrame();
			gl::viewport(getWindowSize());
			mHmd->enableEye(ci::vr::EYE_HMD);
			drawScene();
		}
		else {
			gl::viewport(getWindowSize());
			gl::setMatricesWindow(getWindowSize());
			mHmd->drawMirrored(getWindowBounds(), true);
		}
	}
	else {
		gl::viewport(getWindowSize());
		gl::setMatrices(mCamera);
		drawScene();
	}
	if (mVDSettings->mCursorVisible) {
		//renderUIToFbo();
		//gl::draw(mUIFbo->getColorTexture());
		mVDUI->Run("UI", (int)getAverageFps());

	}


}
// Render the UI into the FBO
//void VideodrommVRApp::renderUIToFbo()
//{
//	if (mVDUI->isReady()) {
//		// this will restore the old framebuffer binding when we leave this function
//		// on non-OpenGL ES platforms, you can just call mFbo->unbindFramebuffer() at the end of the function
//		// but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
//		gl::ScopedFramebuffer fbScp(mUIFbo);
//		// setup the viewport to match the dimensions of the FBO
//		gl::ScopedViewport scpVp(ivec2(0), ivec2(mVDSettings->mFboWidth * mVDSettings->mUIZoom, mVDSettings->mFboHeight * mVDSettings->mUIZoom));
//		gl::clear();
//		gl::color(Color::white());
//
//	}
//	mVDUI->Run("UI", (int)getAverageFps());
//
//}
void prepareSettings(App::Settings *settings)
{
	settings->setTitle("Cinder VR - Videodromm");
	settings->setWindowSize(1920 / 2, 1080 / 2);
}

CINDER_APP(VideodrommVRApp, RendererGl(RendererGl::Options().msaa(0)), prepareSettings)

