#include "ofMain.h"
#include "ofxZmq.h"

// assumes linear gamma
ofColor getAverage(const ofPixels& pix, int x, int y, int w, int h) {
    float r = 0, g = 0, b = 0;
    for(int j = y; j < y + h; j++) {
        for(int i = x; i < x + w; i++) {
            const ofColor& cur = pix.getColor(i, j);
            r += cur.r;
            g += cur.g;
            b += cur.b;
        }
    }
    float n = w * h;
    return ofColor(r / n, g / n, b / n);
}

ofRectangle getCenterRectangle(const ofImage& img) {
    int width = img.getWidth(), height = img.getHeight();
    int side = MIN(width, height);
    ofRectangle crop;
    crop.setFromCenter(width / 2, height / 2, side, side);
    return crop;
}

void getCenterSquare(const ofPixels& img, int w, int h, ofImage& out, float zoom=1, ofImageType imageType=OF_IMAGE_COLOR) {
    ofRectangle box = getCenterRectangle(img);
    box.scaleFromCenter(1 / zoom);
    out.allocate(w, h, imageType);
    int sw = box.getWidth() / w;
    int sh = box.getHeight() / h;
    for(int y = 0; y < h; y++) {
        float sy = ofMap(y, 0, w, box.getTop(), box.getBottom());
        for(int x = 0; x < w; x++) {
            float sx = ofMap(x, 0, w, box.getLeft(), box.getRight());
            ofColor avg = getAverage(img, sx, sy, sw, sh);
            if(imageType == OF_IMAGE_GRAYSCALE) {
                avg = ofColor(avg.getBrightness());
            }
            out.setColor(x, y, avg);
        }
    }
}

class ofApp : public ofBaseApp {
public:
private:
    ofxZmqPublisher pub;
    ofVideoGrabber grabber;
    ofImage center;
    ofXml settings;
    
    void setup() {
        ofSetFrameRate(60);
        ofBackground(0);
        settings.load("settings.xml");
        grabber.setup(settings.getIntValue("camera/width"), settings.getIntValue("camera/height"));
        pub.bind(settings.getValue("zmq/address"));
        pub.setHighWaterMark(2);
    }
    void update() {
        grabber.update();
        if(grabber.isFrameNew()) {
            float zoom = settings.getFloatValue("output/zoom");
            int w = settings.getIntValue("output/width");
            int h = settings.getIntValue("output/height");
            bool color = settings.getBoolValue("output/color");
            getCenterSquare(grabber.getPixels(), w, h, center, zoom, color ? OF_IMAGE_COLOR : OF_IMAGE_GRAYSCALE);
            if(settings.getBoolValue("output/flip")) {
                center.mirror(0, 1);
            }
            pub.send(center.getPixels().getData(), (color ? 3 : 1) * w * h * sizeof(unsigned char));
            center.update();
        }
    }
    void draw() {
        if(center.getTexture().isAllocated()) {
            center.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
            center.draw(0, 0, ofGetWidth(), ofGetWidth());
        }
    }
};

int main() {
    ofSetupOpenGL(480, 480, OF_WINDOW);
    ofRunApp(new ofApp());
}
