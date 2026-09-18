/*
 * The hexagonal FFT example program.
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
import mplot.colourbarvisual;
import mplot.axesvisual;

int main (int argc, char** argv)
{
    // For masking the FFT. Pass an arg to change from the default
    float outrad = 2500.0f;
    if (argc > 1) { outrad = std::stof (argv[1]); }

    // Create the mathplot Visual window
    mplot::Visual v(1600, 1800, "Hexagonal FFT");
    v.setSceneTrans (sm::vec<float,3>{ float{3.87172}, float{2.9696}, float{-17.3745} });
    v.setSceneRotation (sm::quaternion<float>{ float{1}, float{0}, float{0}, float{0} });

    // We create a hexgrid for our image
    sm::hexgrid<float> hg(0.01f, 4.0f, 0.0f);
    // Let's have a circular boundary
    hg.set_circular_boundary (1.0f);

    // Load a rectangular image with the help of mplot::loadpng().
    std::string fn = "../bike256.png";
    sm::vvec<float> image_data;
    sm::vec<unsigned int, 2> dims = mplot::loadpng (fn, image_data);
    std::cout << "Loaded image with dims: " << dims << std::endl;

    // This controls how large the photo will be on the hexgrid
    sm::vec<float,2> image_scale = {2.0f, 2.0f};
    // You can shift the photo with an offset if necessary
    sm::vec<float,2> image_offset = {0.0f, 0.0f};

    // We now resample the square pixel grid onto the hex grid. Takes a few seconds.
    std::cout << "Start resample (please wait)..." << std::endl;
    sm::vvec<float> hex_image_data = sm::algo::hexgrid::resample_image (hg, image_data, dims[0], image_scale, image_offset);
    std::cout << "resample complete" << std::endl;

    // The colour map type for the images
    auto img_cmap = mplot::ColourMapType::GreyscaleInv;
    // Colour map for our FFT graphs/frequency space. CET_D01, CET_D01A, CET_D06, CET_D09, CET_C3,
    // CET_D13, CET_D04, Berlin, Lisbon, Cork and Broc are all good candidates, with light/dark in
    // the middle, and twin tones for +ve/-ve
    auto fft_cmap = mplot::ColourMapType::CET_D09;

    // Store the width and halfwidth of our grid, to place objects neatly into our scene
    const float hgw = hg.width();
    const float hhgw = hgw / 2.0f;
    // An overall offset
    const sm::vec<float> o = sm::vec<float>{-6.0f, 0.0f};

    // Visualize the hex image data on the hexgrid with a HexGridVisual
    auto hgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::point_up>>(&hg, o);
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&hex_image_data);
    hgv->cm.setType (img_cmap);
    hgv->zScale.set_params (0, 0);
    hgv->addLabel ("Input hex image", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    // Carry out the FFT transform with sm::hexfft::fft
    sm::hexfft::fft<float> hfft (&hg, hex_image_data);

#if 0
    // You can equivalently write this:
    sm::hexfft::fft<float> hfft (&hg); // init on construction
    hfft.forward (hex_image_data);     // forward transform when ready
    hfft.forward (hex_image_data);     // forward transform later on same hfft object

    // or
    sm::hexfft::fft<float> hfft;    // Construct uninitialized
    httf.init (&hg);                // Init when ready
    hfft.forward (hex_image_data);  // forward transform when ready
#endif

    // Get some information about the size of the frequency hexgrid. Uscale is a scaling factor to
    // make the frequency grid (which is 1/L units) approximately the same size in mathplot scene
    // coordinates as the image.
    const float myUscale = hfft.Uscale / 1.8f;
    const float fhgw = hfft.hgf->width() * myUscale;
    const float fhhgw = fhgw / 2.0f;

    // Use a common scale across the FFTs
    constexpr bool fixed_colourscale = true;
    sm::scale<float, float> clrscale;
    clrscale.compute_scaling (-1200, 1200);

    // Show the real part of FFT on a hexgrid
    auto fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw, fhgw * 0.6f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setComplexData (&hfft.X_hexgrid);
    fhgv->complexHandling = mplot::complex_number_handling::as_real_scalar;
    if constexpr (fixed_colourscale) { fhgv->colourScale = clrscale; }
    fhgv->cm.setType (fft_cmap);
    fhgv->zScale.null_scaling();
    fhgv->addLabel ("FFT (real component)", sm::vec<float>{-fhhgw, fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    auto fhgvp = v.addVisualModel (fhgv);

    // Axes for the real FFT
    auto tav = std::make_unique<mplot::AxesVisual<float>>(o + sm::vec<float>{ hgw, fhgw * 0.6f } + sm::vec<>{-fhhgw, -fhhgw} );
    tav->set_parent (v.get_id());
    tav->axis_ends = {fhgw, fhgw};
    tav->input_min = {-hfft.hgf->width() / 2.0f, -hfft.hgf->width() / 2.0f, 0};
    tav->input_max = {hfft.hgf->width() / 2.0f, hfft.hgf->width() / 2.0f, 1};
    tav->xlabel = "f_y";
    tav->ylabel = "f_x";
    tav->xaxislabelgap = 0.02f;
    tav->yaxislabelgap = 0.0f;
    tav->fontsize = 0.03f;
    tav->axisstyle = mplot::axisstyle::L;
    tav->finalize();
    v.addVisualModel (tav);

    // Colourbar
    auto cbv = std::make_unique<mplot::ColourBarVisual<float>>(o + sm::vec<float>{ hgw + 1.1f * fhhgw, fhgw * 0.6f + 0.2f });
    cbv->set_parent (v.get_id());
    cbv->orientation = mplot::colourbar_orientation::vertical;
    cbv->tickside = mplot::colourbar_tickside::right_or_below;
    cbv->width = 0.06f;
    cbv->length = 0.4f;
    cbv->framelinewidth = 0.003f;
    // Copy colourmap and scale from the FFT HexGridVisual to this colourbar visual
    cbv->cm = fhgvp->cm;
    cbv->scale = fhgvp->colourScale;
    cbv->finalize();
    v.addVisualModel (cbv);

    // Show the imaginary part of FFT on a hexgrid
    fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw, -fhgw * 0.6f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setComplexData (&hfft.X_hexgrid);
    fhgv->complexHandling = mplot::complex_number_handling::as_imaginary_scalar;
    if constexpr (fixed_colourscale) { fhgv->colourScale = clrscale; }
    fhgv->cm.setType (fft_cmap);
    fhgv->zScale.null_scaling();
    fhgv->addLabel ("FFT (imaginary component)", sm::vec<float>{-fhhgw, fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);

    // Axes
    tav = std::make_unique<mplot::AxesVisual<float>>(o + sm::vec<float>{ hgw, -fhgw * 0.6f } + sm::vec<>{-fhhgw, -fhhgw} );
    tav->set_parent (v.get_id());
    tav->axis_ends = {fhgw, fhgw};
    tav->input_min = {-hfft.hgf->width() / 2.0f, -hfft.hgf->width() / 2.0f, 0};
    tav->input_max = {hfft.hgf->width() / 2.0f, hfft.hgf->width() / 2.0f, 1};
    tav->xlabel = "f_y";
    tav->ylabel = "f_x";
    tav->xaxislabelgap = 0.02f;
    tav->yaxislabelgap = 0.0f;
    tav->fontsize = 0.03f;
    tav->axisstyle = mplot::axisstyle::L;
    tav->finalize();
    v.addVisualModel (tav);

    // Call the inverse method to return the inverse FFT, which should recover the image
    sm::vvec<std::complex<float>> invimg = hfft.inverse();

    // Extract the real component of the returned inverse
    sm::vvec<float> img_r (invimg.size(), 0.0f);
    sm::vvec<float> img_i (invimg.size(), 0.0f);
    for (std::uint32_t i = 0; i < invimg.size(); ++i) {
        img_r[i] = std::real (invimg[i]);
        img_i[i] = std::imag (invimg[i]);
    }

    // Visualize the real component of the inverse FFT - should look the same as the original image
    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, o + sm::vec<float>{2.0f * hgw, 0.0f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&img_r);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.null_scaling();
    hgv->addLabel ("Inverse FFT (real)", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    auto real_cscale = hgv->colourScale;
    v.addVisualModel (hgv);

    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, o + sm::vec<float>{3.0f * hgw, 0.0f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&img_i);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.null_scaling();
    hgv->addLabel ("Inverse FFT (imag, autoscaled)", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, o + sm::vec<float>{4.0f * hgw, 0.0f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&img_i);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.null_scaling();
    hgv->colourScale = real_cscale;
    hgv->addLabel ("Inverse FFT (imag, scaled as real)", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    // Save the X_hexgrid to demonstrate two inverse FFTs after modifying X_hexgrid
    auto X_hexsave = hfft.X_hexgrid;

    // Masking the FFT inside a radius filters out low frequency data from the image
    sm::algo::hexgrid::mask_inside_radius<float, sm::hexalign::flat_up, std::complex<float>> (*hfft.hgf, hfft.X_hexgrid, outrad, std::complex<float>{0.0f, 0.0f});
    sm::vvec<std::complex<float>> invimg_in = hfft.inverse();

    // Show masked-inside FFT
    // real
    fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw, -hgw * 1.5f + fhgw * 0.6f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setComplexData (&hfft.X_hexgrid);
    fhgv->complexHandling = mplot::complex_number_handling::as_real_scalar;
    if constexpr (fixed_colourscale) { fhgv->colourScale = clrscale; }
    fhgv->cm.setType (fft_cmap);
    fhgv->zScale.null_scaling();
    fhgv->addLabel ("FFT masked inside (real)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);
    // imag
    fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw, -hgw * 1.5f - fhgw * 0.6f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setComplexData (&hfft.X_hexgrid);
    fhgv->complexHandling = mplot::complex_number_handling::as_imaginary_scalar;
    if constexpr (fixed_colourscale) { fhgv->colourScale = clrscale; }
    fhgv->cm.setType (fft_cmap);
    fhgv->zScale.null_scaling();
    fhgv->addLabel ("FFT masked inside (imaginary)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);

    // Masking outside the radius filters out high frequency data
    hfft.X_hexgrid = X_hexsave;
    sm::algo::hexgrid::mask_outside_radius<float, sm::hexalign::flat_up, std::complex<float>> (*hfft.hgf, hfft.X_hexgrid, outrad, std::complex<float>{0.0f, 0.0f});
    sm::vvec<std::complex<float>> invimg_out = hfft.inverse();

    // Show masked-outside FFT
    // real
    fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw, -hgw * 3.0f + fhgw * 0.6f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setComplexData (&hfft.X_hexgrid);
    fhgv->complexHandling = mplot::complex_number_handling::as_real_scalar;
    if constexpr (fixed_colourscale) { fhgv->colourScale = clrscale; }
    fhgv->cm.setType (fft_cmap);
    fhgv->zScale.null_scaling();
    fhgv->addLabel ("FFT masked outside (real)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);
    // image
    fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ hgw, -hgw * 3.0f - fhgw * 0.6f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = myUscale;
    fhgv->setComplexData (&hfft.X_hexgrid);
    fhgv->complexHandling = mplot::complex_number_handling::as_imaginary_scalar;
    if constexpr (fixed_colourscale) { fhgv->colourScale = clrscale; }
    fhgv->cm.setType (fft_cmap);
    fhgv->zScale.null_scaling();
    fhgv->addLabel ("FFT masked outside (imaginary)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);

    // Inverse of masked inside
    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, o + sm::vec<float>{2.0f * hgw, -hgw * 1.5f});
    hgv->set_parent (v.get_id());
    hgv->setComplexData (&invimg_in);
    hgv->complexHandling = mplot::complex_number_handling::as_real_scalar;
    hgv->cm.setType (img_cmap);
    hgv->zScale.null_scaling();
    hgv->addLabel ("Inverse FFT masked inside radius (high pass/low masked)", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    // Inverse of masked outside
    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, o + sm::vec<float>{2.0f * hgw, -hgw * 3.0f});
    hgv->set_parent (v.get_id());
    hgv->setComplexData (&invimg_out);
    hgv->complexHandling = mplot::complex_number_handling::as_real_scalar;
    hgv->cm.setType (img_cmap);
    hgv->zScale.null_scaling();
    hgv->addLabel ("Inverse FFT masked outside radius (low pass/ high masked)", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    return 0;
}
