#include <openpose/utilities/fastMath.hpp>
#include <openpose/utilities/openCv.hpp> // resizeGetScaleFactor
#include <openpose/core/scaleAndSizeExtractor.hpp>

namespace op
{
    ScaleAndSizeExtractor::ScaleAndSizeExtractor(const Point<int>& netInputResolution,
                                                 const Point<int>& outputResolution, const int scaleNumber,
                                                 const double scaleGap) :
        mNetInputResolution{netInputResolution},
        mOutputSize{outputResolution},
        mScaleNumber{scaleNumber},
        mScaleGap{scaleGap}
    {
        try
        {
            // Security checks
            if ((netInputResolution.x > 0 && netInputResolution.x % 16 != 0)
                || (netInputResolution.y > 0 && netInputResolution.y % 16 != 0))
                error("Net input resolution must be multiples of 16.", __LINE__, __FUNCTION__, __FILE__);
            if (scaleNumber < 1)
                error("There must be at least 1 scale.", __LINE__, __FUNCTION__, __FILE__);
            if (scaleGap <= 0.)
                error("The gap between scales must be strictly positive.", __LINE__, __FUNCTION__, __FILE__);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    std::tuple<std::vector<double>, std::vector<Point<int>>, double, Point<int>> ScaleAndSizeExtractor::extract(
        const Point<int>& inputResolution) const
    {
        try
        {
            // Security checks
            if (inputResolution.area() <= 0)
                error("Wrong input element (empty cvInputData).", __LINE__, __FUNCTION__, __FILE__);
            // scaleRatios & sizes - Reescale keeping aspect ratio
            std::vector<double> scaleRatios(mScaleNumber, 1.f);
            std::vector<Point<int>> sizes(mScaleNumber);
            for (auto i = 0; i < mScaleNumber; i++)
            {
                const auto currentScale = 1. - i*mScaleGap;
                if (currentScale < 0. || 1. < currentScale)
                    error("All scales must be in the range [0, 1], i.e. 0 <= 1-scale_number*scale_gap <= 1",
                          __LINE__, __FUNCTION__, __FILE__);

                const auto targetWidth = fastTruncate(intRound(mNetInputResolution.x * currentScale) / 16 * 16, 1,
                                                      mNetInputResolution.x);
                const auto targetHeight = fastTruncate(intRound(mNetInputResolution.y * currentScale) / 16 * 16, 1,
                                                       mNetInputResolution.y);
                const Point<int> targetSize{targetWidth, targetHeight};
                scaleRatios[i] = resizeGetScaleFactor(inputResolution, targetSize);
                sizes[i] = mNetInputResolution;
            }
            // scaleInputToOutput - Scale between input and desired output size
            Point<int> outputResolution;
            double scaleInputToOutput;
            // Output = mOutputSize3D size
            if (mOutputSize.x > 0 && mOutputSize.y > 0)
            {
                outputResolution = mOutputSize;
                scaleInputToOutput = resizeGetScaleFactor(inputResolution, outputResolution);
            }
            // Output = input size
            else
            {
                outputResolution = inputResolution;
                scaleInputToOutput = 1.;
            }
            // Return result
            return std::make_tuple(scaleRatios, sizes, scaleInputToOutput, outputResolution);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return std::make_tuple(std::vector<double>{}, std::vector<Point<int>>{}, 1., Point<int>{});
        }
    }
}
