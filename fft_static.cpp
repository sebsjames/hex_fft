/*
 * Static function FFTs
 *
 * \author Seb James
 * \date 2026
 */

#include <memory>
#include <iostream>
#include <string>
#include <complex>

import sm.vec;
import sm.vvec;
import sm.hexfft;
import sm.hexgrid;
import sm.algo.hexgrid;
import sm.grid;

import mplot.loadpng;
import mplot.visual;
import mplot.hexgridvisual;
import mplot.gridvisual;

// Helper function to draw one group of function + FFT graphs
void draw_set (mplot::Visual<>& v, const sm::vec<float>& o, const std::string& fn_name,
               sm::hexgrid<float>& hg, sm::hexfft::fft<float>& hfft, sm::vvec<float>& hex_image_data, bool flatf = false)
{
    // Store the width and halfwidth of our grid, to place objects neatly into our scene
    const float hgw = hg.width();
    const float hhgw = hgw / 2.0f;

    // Visualize the function
    auto hgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::point_up>>(&hg, o);
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&hex_image_data);
    hgv->hexVisMode = mplot::HexVisMode::HexInterp;
    hgv->cm.setType (mplot::ColourMapType::Ice);
    if (flatf) { hgv->zScale.null_scaling(); }
    hgv->addLabel (fn_name, sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    // Carry out the FFT transform with sm::hexfft::fft
    hfft.forward (hex_image_data);

    // Extract real and imaginary components into vvecs for visualization
    sm::vvec<float> fft_r (hfft.X_hexgrid.size());
    sm::vvec<float> fft_i (hfft.X_hexgrid.size());
    for (std::uint32_t i = 0; i < fft_r.size(); ++i) {
        fft_r[i] = std::real(hfft.X_hexgrid[i]);
        fft_i[i] = std::imag(hfft.X_hexgrid[i]);
    }

    // Get some information about the size of the frequency hexgrid. Uscale is a scaling factor to
    // make the frequency grid (which is 1/L units) approximately the same size in mathplot scene
    // coordinates as the image.
    const float myUscale = hfft.Uscale / 1.8f;
    const float fhgw = hfft.hgf->width() * myUscale;
    const float fhhgw = fhgw / 2.0f;

    // Show the real part of FFT on a hexgrid
    auto fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw, 0.0f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setScalarData (&fft_r);
    fhgv->colourScale.compute_scaling (-900, 1200);
    fhgv->cm.setType (mplot::ColourMapType::CET_D09);
    fhgv->hexVisMode = mplot::HexVisMode::HexInterp;
    //fhgv->zScale.null_scaling();
    //fhgv->zScale.compute_scaling (0, 1200);
    fhgv->addLabel ("FFT (real component)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);

    // Show the imaginary part of FFT on a hexgrid
    fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw + 1.2f * fhgw, 0.0f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setScalarData (&fft_i);
    fhgv->colourScale.compute_scaling (-900, 1200);
    fhgv->cm.setType (mplot::ColourMapType::CET_D09);
    fhgv->hexVisMode = mplot::HexVisMode::HexInterp;
    //fhgv->zScale.null_scaling();
    fhgv->zScale.compute_scaling (0, 1200);
    fhgv->addLabel ("FFT (imaginary component)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);
}

int main (int argc, char** argv)
{
    using mc = sm::mathconst<float>;

    // Create the mathplot Visual window
    mplot::Visual v(1600, 1800, "Dynamic FFT");
    v.setSceneTrans (sm::vec<float,3>{ float{3.87172}, float{2.9696}, float{-17.3745} });
    v.rotateAboutNearest (true);

    // We create a hexgrid for our image
    sm::hexgrid<float> hg(0.01f, 4.0f, 0.0f);

    hg.set_rectangular_boundary (2.0f, 2.0f);
    // Experiment with the circular boundary - you can see the artefacts from the boundary in the FFTs
    //hg.set_circular_boundary (1.0f);

    // Function data container
    sm::vvec<float> data (hg.num());

    // Initialize our FFT object
    sm::hexfft::fft<float> hfft (&hg);

    // First function: circularly symmetric sine, radially decreasing
    sm::vvec<float> r(hg.num(), 0.0f);
    float k = 4.0f;
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        r[ri] = std::sqrt (hg.d_x[ri] * hg.d_x[ri] + hg.d_y[ri] * hg.d_y[ri]);
        data[ri] = std::sin (k * r[ri]) / k * r[ri];
    }

    const sm::vec<float> o0 = sm::vec<float>{-6.0f, 0.0f};
    sm::vec<float> o = o0;
    draw_set (v, o, "Decaying sine, k = 4", hg, hfft, data);

    // Circ symmetric sine, change k
    k = 16.0f;
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        r[ri] = std::sqrt (hg.d_x[ri] * hg.d_x[ri] + hg.d_y[ri] * hg.d_y[ri]);
        data[ri] = std::sin (k * r[ri]) / k * r[ri];
    }
    o[1] -= 2.2f;
    draw_set (v, o, "Decaying sine, k = 16", hg, hfft, data);

    // Horz sine
    for (unsigned int ri = 0; ri < hg.num(); ++ri) { data[ri] = 0.2f * std::sin (k * hg.d_x[ri]); }
    o[1] -= 2.2f;
    draw_set (v, o, "Horz sine, k = 16", hg, hfft, data);

    // Vert sine
    for (unsigned int ri = 0; ri < hg.num(); ++ri) { data[ri] = 0.2f * std::sin (k * hg.d_y[ri]); }
    o[1] -= 2.2f;
    draw_set (v, o, "Vert sine, k = 16", hg, hfft, data);

    // Diagonal sine
    float m = std::sqrt (1.0f + std::tan(45.0f * mc::deg2rad) * std::tan(45.0f * mc::deg2rad));
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        float f = hg.d_y[ri] + hg.d_x[ri];
        data[ri] = 0.2f * std::sin (k / m * f);
    }
    o[1] -= 2.2f;
    draw_set (v, o, "45deg sine, k = 16", hg, hfft, data);

    // Diagonal sine
    m = std::sqrt (1.0f + std::tan(30.0f * mc::deg2rad) * std::tan(30.0f * mc::deg2rad));
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        float f = std::tan(30.0f * mc::deg2rad) * hg.d_y[ri] + hg.d_x[ri];
        data[ri] = 0.2f * std::sin (k / m * f);
    }
    o[1] -= 2.2f;
    draw_set (v, o, "30deg sine, k = 16", hg, hfft, data);

    // Diagonal sine
    m = std::sqrt (1.0f + std::tan(60.0f * mc::deg2rad) * std::tan(60.0f * mc::deg2rad));
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        float f = std::tan(60.0f * mc::deg2rad) * hg.d_y[ri] + hg.d_x[ri];
        data[ri] = 0.2f * std::sin (k / m * f);
    }
    // New col
    o = o0 + sm::vec<float>{6.3f};
    draw_set (v, o, "60deg sine, k = 16", hg, hfft, data);

    // Diagonal sine, low freq
    k = 4.0f;
    m = std::sqrt (1.0f + std::tan(60.0f * mc::deg2rad) * std::tan(60.0f * mc::deg2rad));
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        float f = std::tan(60.0f * mc::deg2rad) * hg.d_y[ri] + hg.d_x[ri];
        data[ri] = 0.2f * std::sin (k / m * f);
    }
    o[1] -= 2.2f;
    draw_set (v, o, "60deg sine, low freq (k = 4)", hg, hfft, data);

    // Diagonal sine, high freq
    k = 64.0f;
    m = std::sqrt (1.0f + std::tan(60.0f * mc::deg2rad) * std::tan(60.0f * mc::deg2rad));
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        float f = std::tan(60.0f * mc::deg2rad) * hg.d_y[ri] + hg.d_x[ri];
        data[ri] = 0.2f * std::sin (k / m * f);
    }
    o[1] -= 2.2f;
    draw_set (v, o, "60deg sine, high freq (k = 64)", hg, hfft, data);

    // Diagonal sine, high freq near limit
    k = 256.0f;
    m = std::sqrt (1.0f + std::tan(30.0f * mc::deg2rad) * std::tan(30.0f * mc::deg2rad));
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        float f = std::tan(30.0f * mc::deg2rad) * hg.d_y[ri] + hg.d_x[ri];
        data[ri] = 0.2f * std::sin (k / m * f);
    }
    o[1] -= 2.2f;
    draw_set (v, o, "30deg sine, V high freq (k = 256)", hg, hfft, data);

    // Manually set nyquist limit
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        // Set value from modulus of the gi index in the hexgrid
        data[ri] = ((2 + ((hg.d_gi[ri]) % 2)) % 2)  == 0 ? 0.2f : -0.2f;
    }
    o[1] -= 2.2f;
    draw_set (v, o, "Freq limit (gi)", hg, hfft, data, true);

    // Manually set nyquist limit
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        // Set value from modulus of the gi index in the hexgrid
        data[ri] = ((2 + ((hg.d_ri[ri]) % 2)) % 2)  == 0 ? 0.2f : -0.2f;
    }
    o[1] -= 2.2f;
    draw_set (v, o, "(-30deg), Freq limit (ri)", hg, hfft, data, true);

    // Manually set nyquist limit
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        // Set value from modulus of the gi index in the hexgrid
        auto bi = hg.d_gi[ri] - hg.d_ri[ri];
        data[ri] = ((2 + (bi % 2)) % 2) == 0 ? 0.2f : -0.2f;
    }
    o = o0 + sm::vec<float>{2 * 6.3f};
    draw_set (v, o, "(30deg), Freq limit (bi)", hg, hfft, data, true);

    // Diagonal sine always increasing
    k = 16.0f;
    m = std::sqrt (1.0f + std::tan(60.0f * mc::deg2rad) * std::tan(60.0f * mc::deg2rad));
    float sum = 0.0f;
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        float f = std::tan(60.0f * mc::deg2rad) * hg.d_y[ri] + hg.d_x[ri];
        data[ri] = 0.02f * std::sin (k / m * f) + 0.04f * f;
        sum += data[ri];
    }
    // so that mean is 0
    for (unsigned int ri = 0; ri < hg.num(); ++ri) { data[ri] -= sum / 2.0f; }
    o[1] -= 2.2f;
    draw_set (v, o, "60deg sine increasing, k = 16", hg, hfft, data);

    // Diagonal sine increasing, but with more power in the high freq.
    k = 16.0f;
    m = std::sqrt (1.0f + std::tan(60.0f * mc::deg2rad) * std::tan(60.0f * mc::deg2rad));
    sum = 0.0f;
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        float f = std::tan(60.0f * mc::deg2rad) * hg.d_y[ri] + hg.d_x[ri];
        data[ri] = 0.1f * std::sin (k / m * f) + 0.04f * f;
        sum += data[ri];
    }
    // so that mean is 0
    for (unsigned int ri = 0; ri < hg.num(); ++ri) { data[ri] -= sum / 2.0f; }
    o[1] -= 2.2f;
    draw_set (v, o, "60deg sine increasing, k = 16", hg, hfft, data);

    v.keepOpen();

    return 0;
}
