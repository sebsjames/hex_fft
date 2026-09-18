/*
 * Dynamic FFTs
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

int main (int argc, char** argv)
{
    // Create the mathplot Visual window
    mplot::Visual v(1600, 1800, "Dynamic FFT");
    v.setSceneTrans (sm::vec<float,3>{ float{3.87172}, float{2.9696}, float{-17.3745} });
    v.setSceneRotation (sm::quaternion<float>{ float{1}, float{0}, float{0}, float{0} });

    // We create a hexgrid for our image
    sm::hexgrid<float> hg(0.01f, 4.0f, 0.0f);
    // Let's have a circular boundary
    hg.set_circular_boundary (1.0f);

    sm::vvec<float> hex_image_data (hg.num());
    sm::vvec<float> r(hg.num(), 0.0f);
    float k = 2.0f;
    for (unsigned int ri = 0; ri < hg.num(); ++ri) {
        r[ri] = std::sqrt (hg.d_x[ri] * hg.d_x[ri] + hg.d_y[ri] * hg.d_y[ri]);
        hex_image_data[ri] = std::sin (k * r[ri]) / k * r[ri];
    }

    // Store the width and halfwidth of our grid, to place objects neatly into our scene
    const float hgw = hg.width();
    const float hhgw = hgw / 2.0f;
    // An overall offset
    const sm::vec<float> o = sm::vec<float>{-6.0f, 0.0f};

    // Visualize the hex image data on the hexgrid with a HexGridVisual
    auto hgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::point_up>>(&hg, o);
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&hex_image_data);
    hgv->hexVisMode = mplot::HexVisMode::Triangles;
    hgv->cm.setType (mplot::ColourMapType::Ice);
    hgv->zScale.null_scaling();
    hgv->addLabel ("Input hex image", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    auto hgvp = v.addVisualModel (hgv);

    // Carry out the FFT transform with sm::hexfft::fft
    sm::hexfft::fft<float> hfft (&hg, hex_image_data);

    // Get some information about the size of the frequency hexgrid. Uscale is a scaling factor to
    // make the frequency grid (which is 1/L units) approximately the same size in mathplot scene
    // coordinates as the image.
    const float myUscale = hfft.Uscale / 1.8f;
    const float fhgw = hfft.hgf->width() * myUscale;
    const float fhhgw = fhgw / 2.0f;

    // Show the real part of FFT on a hexgrid
    auto fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw, fhgw * 0.6f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setComplexData (&hfft.X_hexgrid);
    fhgv->complexHandling = mplot::complex_number_handling::as_real_scalar;
    fhgv->colourScale.compute_scaling (-900, 1200);
    fhgv->cm.setType (mplot::ColourMapType::Ice);
    fhgv->hexVisMode = mplot::HexVisMode::Triangles;
    fhgv->zScale.null_scaling();
    fhgv->addLabel ("FFT (real component)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    auto r_fftp = v.addVisualModel (fhgv);

    // Show the imaginary part of FFT on a hexgrid
    fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw, -fhgw * 0.6f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setComplexData (&hfft.X_hexgrid);
    fhgv->complexHandling = mplot::complex_number_handling::as_imaginary_scalar;
    fhgv->colourScale.compute_scaling (-900, 1200);
    fhgv->cm.setType (mplot::ColourMapType::Ice);
    fhgv->hexVisMode = mplot::HexVisMode::Triangles;
    fhgv->zScale.null_scaling();
    fhgv->addLabel ("FFT (imaginary component)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    auto i_fftp = v.addVisualModel (fhgv);


    // Loop and recompute the function, then re-FFT it
    while (v.readyToFinish() == false) {

        v.poll();
        if (k > 18.0f) { k = 1.0f; }

#pragma omp parallel for shared(r,k,data)
        for (unsigned int hi = 0; hi < hg.num(); ++hi) {
            hex_image_data[hi] = std::sin (k * r[hi]) / k * r[hi];
        }

        hfft.forward (hex_image_data);
        hgvp->reinitColours();
        r_fftp->reinitColours();
        i_fftp->reinitColours();

        k += 0.2f;

        v.render();
    }

    return 0;
}
