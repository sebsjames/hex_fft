/*
 * FFT on a hexgrid, which happens to be rectangular in shape.
 *
 * This debugging program shows internal data grids for the fft, including the ASA grids d0, d1
 * (image space) and X0/X1 (frequency space).
 *
 * If you're interested in using the hexfft code, focus on the other program, fft_play.cpp
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

    sm::hexgrid<float, sm::hexalign::point_up> hgc(0.01f, 4.0f, 0.0f);
    hgc.set_circular_boundary (1.0f);

    sm::hexgrid<float, sm::hexalign::point_up> hgr(0.01f, 4.0f, 0.0f);
    hgr.set_rectangular_boundary (2.0f, 2.0f);

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
    std::cout << "Start circular resample..." << std::endl;
    sm::vvec<float> circ_image_data = sm::algo::hexgrid::resample_image (hgc, image_data, dims[0], image_scale, image_offset);
    std::cout << "Start rectangular resample..." << std::endl;
    sm::vvec<float> rect_image_data = sm::algo::hexgrid::resample_image (hgr, image_data, dims[0], image_scale, image_offset);
    std::cout << "resample complete" << std::endl;

    // Store the width and halfwidth of our grid, to place objects neatly into our scene
    const float chgw = hgc.width();
    const float chhgw = chgw / 2.0f;
    const float rhgw = hgr.width();
    const float rhhgw = rhgw / 2.0f;

    auto hgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::point_up>>(&hgc, sm::vec<float>{-3.5f, 3.8f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&circ_image_data);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.set_params (0, 0);
    hgv->addLabel ("Input hex image", sm::vec<float>{-chhgw, -chhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    hgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::point_up>>(&hgr, sm::vec<float>{-5.8f, 3.8f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&rect_image_data);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.set_params (0, 0);
    hgv->addLabel ("Input hex image", sm::vec<float>{-rhhgw, -rhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    // Transform with FFT. The second template argument to hexfft::fft turns on the creation of
    // hg_asa, which is used to visualize the effective, regular rectangular hexgrid that is placed
    // around the input, arbitrarily shaped hexgrid before splitting the image into two rectangular
    // ASA grids.
    sm::hexfft::fft<float, true> hfft (&hgc, circ_image_data);
    std::cout << "hfft (circ) cols = " << hfft.asa_cols << ", and rows = " << hfft.asa_rows << std::endl;

    sm::hexfft::fft<float, true> hfft_rect (&hgr, rect_image_data);
    std::cout << "hfft_rect cols = " << hfft_rect.asa_cols << ", and rows = " << hfft_rect.asa_rows << std::endl;

    sm::vvec<float> fft_r (hfft.X_hexgrid.size());
    sm::vvec<float> fft_i (hfft.X_hexgrid.size());
    for (std::uint32_t i = 0; i < fft_r.size(); ++i) {
        fft_r[i] = std::real(hfft.X_hexgrid[i]);
        fft_i[i] = std::imag(hfft.X_hexgrid[i]);
    }

    // rows/cols:
    constexpr sm::vec<float, 2> grid_spacing = {0.01f, 0.01f};
    constexpr sm::vec<float, 2> null_offset = {0.0f, 0.0f};
    sm::grid<std::uint32_t, float> grid(hfft.asa_cols, hfft.asa_rows, grid_spacing, null_offset,
                                        sm::griddomainwrap::none,
                                        sm::gridorder::bottomleft_to_topright_colmaj);

    sm::grid<std::uint32_t, float> grid_r(hfft_rect.asa_cols, hfft_rect.asa_rows, grid_spacing, null_offset,
                                          sm::griddomainwrap::none,
                                          sm::gridorder::bottomleft_to_topright_colmaj);

    sm::vvec<float> d0 (hfft.d0.size());
    sm::vvec<float> d1 (hfft.d0.size());
    sm::vvec<float> d0r (hfft_rect.d0.size());
    sm::vvec<float> d1r (hfft_rect.d0.size());
    sm::vvec<float> X0 (hfft.X0.size());
    sm::vvec<float> X1 (hfft.X0.size());

    for (std::uint32_t i = 0; i < d0.size(); ++i) {
        d0[i] = std::real (hfft.d0[i]);
        d1[i] = std::real (hfft.d1[i]);
        X0[i] = std::real (hfft.X0[i]);
        X1[i] = std::real (hfft.X1[i]);
    }
    for (std::uint32_t i = 0; i < d0r.size(); ++i) {
        d0r[i] = std::real (hfft_rect.d0[i]);
        d1r[i] = std::real (hfft_rect.d1[i]);
    }
    std::cout << "X0 mean/sd/range: " << X0.mean() << ", " << X0.std() << ", " << X0.range() << std::endl;
    std::cout << "X1 mean/sd/range: " << X1.mean() << ", " << X1.std() << ", " << X1.range() << std::endl;

    float hshift1 = 0.75f;
    // Grid 1 ds.first
    auto gv = std::make_unique<mplot::GridVisual<float>>(&grid, sm::vec<float>{-4.5f, 0.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&d0);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    gv->addLabel ("circ d0 (odd input rows)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    gv = std::make_unique<mplot::GridVisual<float>>(&grid, sm::vec<float>{-4.5f, 2.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&d1);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    gv->addLabel ("circ d1 (even)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    gv = std::make_unique<mplot::GridVisual<float>>(&grid_r, sm::vec<float>{-6.8f, 0.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&d0r);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    gv->addLabel ("rect d0 (odd input rows)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    gv = std::make_unique<mplot::GridVisual<float>>(&grid_r, sm::vec<float>{-6.8f, 2.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&d1r);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    gv->addLabel ("rect d1 (even)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // Viz the ASA-compliant hexgrid
    auto hgv1 = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::point_up, mplot::gl::version_4_1>>(hfft.hg_asa.get(), sm::vec<float>{-4.5f, -2.0f - 2 * hshift1});
    hgv1->set_parent (v.get_id());
    hgv1->cm.setType (mplot::ColourMapType::GreyscaleInv);
    //hgv1->showboundary = true;
    hgv1->zScale.null_scaling();
    hgv1->setScalarData (&hfft.data_asa_real);
    hgv1->addLabel ("ASA hexgrid", sm::vec<>{ 0.0f, -hfft.hg_asa->width()/1.8f }, mplot::TextFeatures(0.02f));
    hgv1->finalize();
    v.addVisualModel (hgv1);

    hgv1 = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::point_up, mplot::gl::version_4_1>>(hfft_rect.hg_asa.get(), sm::vec<float>{-6.8f, -2.0f - 2 * hshift1});
    hgv1->set_parent (v.get_id());
    hgv1->cm.setType (mplot::ColourMapType::GreyscaleInv);
    //hgv1->showboundary = true;
    hgv1->zScale.null_scaling();
    hgv1->setScalarData (&hfft_rect.data_asa_real);
    hgv1->addLabel ("ASA hexgrid", sm::vec<>{ 0.0f, -hfft.hg_asa->width()/1.8f }, mplot::TextFeatures(0.02f));
    hgv1->finalize();
    v.addVisualModel (hgv1);

    // FFT
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, sm::vec<float>{-2.0f, 0.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&X0);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::Ice);
    gv->addLabel ("X0 (odd, re-quadranted)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    gv = std::make_unique<mplot::GridVisual<float>>(&grid, sm::vec<float>{-2.0f, 2.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&X1);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::Ice);
    gv->addLabel ("X1 (even, re-quadranted)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // Get some information about the size of the frequency hexgrid. Uscale is a scaling factor to
    // make the frequency grid (which is 1/L units) approximately the same size in mathplot scene
    // coordinates as the image.
    const float fhgw = hfft.hgf->width() * hfft.Uscale;
    const float fhhgw = fhgw / 2.0f;

    // Real part of FFT on a hexgrid
    auto fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), sm::vec<float>{2.5f, 1.0f});
    fhgv->set_parent (v.get_id());
    fhgv->zoom = (hfft.Uscale);
    fhgv->setScalarData (&fft_r);
    fhgv->colourScale.compute_scaling (-900, 1200);
    fhgv->cm.setType (mplot::ColourMapType::Ice);
    fhgv->zScale.set_params (0, 0);
    fhgv->addLabel ("FFT (real component)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);

    // Imaginary part
    fhgv = std::make_unique<mplot::HexGridVisual<float, sm::hexalign::flat_up>>(hfft.hgf.get(), sm::vec<float>{2.5f, -2.0f});
    fhgv->set_parent (v.get_id());
    fhgv->zoom = (hfft.Uscale);
    fhgv->setScalarData (&fft_i);
    fhgv->colourScale.compute_scaling (-900, 1200);
    fhgv->cm.setType (mplot::ColourMapType::Ice);
    fhgv->zScale.set_params (0, 0);
    fhgv->addLabel ("FFT (imaginary component)", sm::vec<float>{-fhhgw, -fhhgw * 1.1f}, mplot::TextFeatures(0.05f));
    fhgv->finalize();
    v.addVisualModel (fhgv);

    auto X_hexsave = hfft.X_hexgrid;

    // Modify data on hexgrid?
    sm::algo::hexgrid::mask_inside_radius<float, sm::hexalign::flat_up, std::complex<float>> (*hfft.hgf, hfft.X_hexgrid, outrad, std::complex<float>{0.0f, 0.0f});
    // Reconstruct with inverse FFT
    sm::vvec<std::complex<float>> invimg = hfft.inverse();

    hfft.X_hexgrid = X_hexsave;
    sm::algo::hexgrid::mask_outside_radius<float, sm::hexalign::flat_up, std::complex<float>> (*hfft.hgf, hfft.X_hexgrid, outrad, std::complex<float>{0.0f, 0.0f});
    sm::vvec<std::complex<float>> invimg_out = hfft.inverse();


    // Re-show the X0/X1 grids
    for (std::uint32_t i = 0; i < hfft.X0.size(); ++i) {
        d0[i] = std::real (hfft.d0[i]);
        d1[i] = std::real (hfft.d1[i]);
        X0[i] = std::real (hfft.X0[i]);
        X1[i] = std::real (hfft.X1[i]);
    }

    // FFT
    gv = std::make_unique<mplot::GridVisual<float>>(&grid, sm::vec<float>{4.0f, 0.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&X0);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::Ice);
    gv->addLabel ("Low-pass X0 (odd, de-quadranted for inverse)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    gv = std::make_unique<mplot::GridVisual<float>>(&grid, sm::vec<float>{4.0f, 2.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&X1);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::Ice);
    gv->addLabel ("Low-pass X1 (even, de-quadranted for inverse)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    gv = std::make_unique<mplot::GridVisual<float>>(&grid, sm::vec<float>{7.0f, 0.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&d0);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    gv->addLabel ("d0 (odd input rows)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    gv = std::make_unique<mplot::GridVisual<float>>(&grid, sm::vec<float>{7.0f, 2.0f - hshift1});
    gv->set_parent (v.get_id());
    gv->gridVisMode = mplot::GridVisMode::RectInterp;
    gv->setScalarData (&d1);
    gv->zScale.set_params (0, 0);
    gv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    gv->addLabel ("d1 (even)", sm::vec<float>({0,-0.2,0}), mplot::TextFeatures(0.05f));
    gv->finalize();
    v.addVisualModel (gv);

    // Reconstructed
    sm::vvec<float> img_r (invimg.size(), 0.0f);
    for (std::uint32_t i = 0; i < invimg.size(); ++i) { img_r[i] = std::real (invimg[i]); }

    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hgc, sm::vec<float>{6.5f, -3.0f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&img_r);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.set_params (0, 0);
    hgv->addLabel ("FFT masked inside radius", sm::vec<float>({-0.75,-1.2,0}), mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    sm::vvec<float> img_rout (invimg_out.size(), 0.0f);
    for (std::uint32_t i = 0; i < invimg.size(); ++i) { img_rout[i] = std::real (invimg_out[i]); }

    hgv = std::make_unique<mplot::HexGridVisual<float>>(&hgc, sm::vec<float>{8.5f, -3.0f});
    hgv->set_parent (v.get_id());
    hgv->setScalarData (&img_rout);
    hgv->cm.setType (mplot::ColourMapType::GreyscaleInv);
    hgv->zScale.set_params (0, 0);
    hgv->addLabel ("FFT masked outside radius", sm::vec<float>({-0.75,-1.2,0}), mplot::TextFeatures(0.05f));
    hgv->finalize();
    v.addVisualModel (hgv);

    v.keepOpen();

    return 0;
}
