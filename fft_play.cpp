/*
 * The hexagonal FFT
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
    float outrad = 1000.0f;
    if (argc > 1) {
        outrad = std::stof (argv[1]);
    }

    mplot::Visual v(1600, 1000, "Hexagonal FFT");

    // A hexgrid for our image
    sm::hexgrid<float, sm::hexalign::point_up> hg(0.01f, 4.0f, 0.0f);
    //hg.set_rectangular_boundary (2.0f, 2.0f);
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

    // Here's the hexgrid method that will resample the square pixel grid onto the hex grid
    std::cout << "Start resample (please wait)..." << std::endl;
    sm::vvec<float> hex_image_data = sm::algo::hexgrid::resample_image (hg, image_data, dims[0], image_scale, image_offset);
    std::cout << "resample complete" << std::endl;

    float hgw = hg.width();
    float hhgw = hgw / 2.0f;
    sm::vec<float> o = sm::vec<float>{-6.0f, 0.0f};

    auto hgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::point_up>>(&hg, o);
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&hex_image_data);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.set_params (0, 0);
    hgv->addLabel ("Input hex image", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    // Transform with sm::hexfft
    sm::hexfft::fft<float, true> hfft;
    hfft.init (&hg);
    hfft.forward (hex_image_data);

    std::cout << "fft_data cols = " << hfft.asa_cols << ", and rows = " << hfft.asa_rows << std::endl;

    sm::vvec<float> fft_r (hfft.X_hexgrid.size());
    sm::vvec<float> fft_i (hfft.X_hexgrid.size());
    for (std::uint32_t i = 0; i < fft_r.size(); ++i) {
        fft_r[i] = std::real(hfft.X_hexgrid[i]);
        fft_i[i] = std::imag(hfft.X_hexgrid[i]);
    }

    // How big is the freq hexgrid?
    float fhgw = hfft.hgf->width() * hfft.Uscale;
    float fhhgw = fhgw / 2.0f;

    // Real part of FFT on a hexgrid
    auto fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ 1.5f * hgw, 0.0f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = (hfft.Uscale);
    fhgv->setScalarData (&fft_r);
    fhgv->colourScale.compute_scaling (-900, 1200);
    fhgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    fhgv->zScale.set_params (0, 0);
    fhgv->addLabel ("FFT (real component)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);

    // Imaginary part
    fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), o + sm::vec<float>{ 1.5f * hgw, -hgw * 1.5f });
    fhgv->set_parent (v.get_id());
    fhgv->zoom = (hfft.Uscale);
    fhgv->setScalarData (&fft_i);
    fhgv->colourScale.compute_scaling (-900, 1200);
    fhgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    fhgv->zScale.set_params (0, 0);
    fhgv->addLabel ("FFT (imaginary component)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);

    // The inverse FFT recovers the image
    sm::vvec<std::complex<float>> invimg = hfft.inverse();

    sm::vvec<float> img_r (invimg.size(), 0.0f);
    for (std::uint32_t i = 0; i < invimg.size(); ++i) { img_r[i] = std::real (invimg[i]); }
    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, o + sm::vec<float>{2.8f * hgw, 0.0f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&img_r);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.set_params (0, 0);
    hgv->addLabel ("Inverse FFT", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    // Save the X_hexgrid to make two inverse FFTs after modifying X_hexgrid
    auto X_hexsave = hfft.X_hexgrid;

    // Masking the FFT inside a radius filters out low frequency data from the image
    sm::algo::hexgrid::mask_inside_radius<float, sm::hexalign::flat_up, std::complex<float>> (*hfft.hgf, hfft.X_hexgrid, outrad, std::complex<float>{0.0f, 0.0f});
    sm::vvec<std::complex<float>> invimg_in = hfft.inverse();

    // Masking outside the radius filters out high frequency data
    hfft.X_hexgrid = X_hexsave;
    sm::algo::hexgrid::mask_outside_radius<float, sm::hexalign::flat_up, std::complex<float>> (*hfft.hgf, hfft.X_hexgrid, outrad, std::complex<float>{0.0f, 0.0f});
    sm::vvec<std::complex<float>> invimg_out = hfft.inverse();

    // Reconstructed
    sm::vvec<float> img_rin (invimg_in.size(), 0.0f);
    for (std::uint32_t i = 0; i < invimg_in.size(); ++i) { img_rin[i] = std::real (invimg_in[i]); }

    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, o + sm::vec<float>{3.8f * hgw, 0.0f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&img_rin);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.set_params (0, 0);
    hgv->addLabel ("FFT masked inside radius (high pass/low masked)", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    sm::vvec<float> img_rout (invimg_out.size(), 0.0f);
    for (std::uint32_t i = 0; i < invimg_out.size(); ++i) { img_rout[i] = std::real (invimg_out[i]); }

    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hg, o + sm::vec<float>{3.8f * hgw, -hgw * 1.2f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&img_rout);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.set_params (0, 0);
    hgv->addLabel ("FFT masked outside radius (low pass/ high masked)", sm::vec<float>{-hhgw, -hhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    return 0;
}
