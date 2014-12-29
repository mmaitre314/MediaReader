#pragma once

namespace MediaCaptureReader
{
    [WFM::WebHostHidden]
    public ref class MediaSamplePresenter sealed
    {
    public:

        ///<summary>Creates a sample presenter with format Bgra8.</summary>
        static MediaSamplePresenter^ CreateFromSurfaceImageSource(
            _In_ WUXMI::SurfaceImageSource^ image,
            _In_ MediaGraphicsDevice^ device,
            _In_ int width,
            _In_ int height
            );

        ///<summary>Creates a sample presenter with format Bgra8 and resolution SwapChainPanel.Width x SwapChainPanel.Height.</summary>
        static MediaSamplePresenter^ CreateFromSwapChainPanel(
            _In_ WUXC::SwapChainPanel^ panel,
            _In_ MediaGraphicsDevice^ device,
            _In_ int width,
            _In_ int height
            );
        
        // IClosable
        virtual ~MediaSamplePresenter();

        void Present(MediaSample^ sample);

    private:

        MediaSamplePresenter(_In_ int width, _In_ int height)
            : _width(width)
            , _height(height)
        {
        }

        void _PresentToSurfaceImageSource(const MW::ComPtr<IMFDXGIBuffer>& dxgiBuffer);
        void _PresentToSwapChainPanel(const MW::ComPtr<IMFDXGIBuffer>& dxgiBuffer);

        MW::ComPtr<ISurfaceImageSourceNative> _image;
        MW::ComPtr<ISwapChainPanelNative> _panel;
        MW::ComPtr<IDXGISwapChain1> _swapChain;
        MW::ComPtr<IDXGIDevice> _device;

        int _width;
        int _height;
    };
}
