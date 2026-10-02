/* platform_ios.m - Le due cose che SDL2 non espone su iOS: area sicura (notch, angoli, barra home)
 * e vibrazione leggera dei tasti touch. Compilato solo per iOS / iPadOS. */
#import <UIKit/UIKit.h>

static UIWindow *FinestraAttiva(void) {
    if (@available(iOS 13.0, *)) {
        for (UIScene *scena in UIApplication.sharedApplication.connectedScenes) {
            if (![scena isKindOfClass:[UIWindowScene class]]) continue;
            for (UIWindow *w in ((UIWindowScene *)scena).windows)
                if (w.isKeyWindow) return w;
        }
    }
    return UIApplication.sharedApplication.windows.firstObject;
}

/* Inset in pixel fisici (punti x scala nativa), stesso spazio del drawable OpenGL */
void PlatformIOS_SafeArea(float *l, float *t, float *r, float *b) {
    *l = *t = *r = *b = 0.0f;
    UIWindow *w = FinestraAttiva();
    if (!w) return;
    UIEdgeInsets in = w.safeAreaInsets;
    CGFloat s = w.screen.nativeScale;
    *l = (float)(in.left * s);
    *t = (float)(in.top * s);
    *r = (float)(in.right * s);
    *b = (float)(in.bottom * s);
}

void PlatformIOS_Haptic(void) {
    static UIImpactFeedbackGenerator *gen;
    if (!gen) gen = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleLight];
    [gen impactOccurred];
}
