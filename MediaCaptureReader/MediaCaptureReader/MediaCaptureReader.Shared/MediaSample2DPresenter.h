#pragma once

namespace MediaCaptureReader
{
    [WFM::WebHostHidden]
    public ref class MediaSample2DPresenter sealed
    {
    public:

        ///<summary>Creates a sample presenter with format Bgra8.</summary>
        static MediaSample2DPresenter^ CreateFromSurfaceImageSource(
            _In_ WUXMI::SurfaceImageSource^ image,
            _In_ MediaGraphicsDevice^ device,
            _In_ int width,
            _In_ int height
            );

        ///<summary>Creates a sample presenter with format Bgra8 and resolution SwapChainPanel.Width x SwapChainPanel.Height.</summary>
        static MediaSample2DPresenter^ CreateFromSwapChainPanel(
            _In_ WUXC::SwapChainPanel^ panel,
            _In_ MediaGraphicsDevice^ device,
            _In_ int width,
            _In_ int height
            );
        
        // IClosable
        virtual ~MediaSample2DPresenter();

        void Present(MediaSample2D^ sample);

    private:

        MediaSample2DPresenter(_In_ int width, _In_ int height)
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
