#pragma once

namespace MediaCaptureReader
{
    [WFM::WebHostHidden]
    public ref class MediaSamplePresenter sealed
    {
    public:

        ///<summary>Creates a sample presenter with format Bgra8.</summary>
        static MediaSamplePresenter^ CreateFromSurfaceImageSource(
            WUXMI::SurfaceImageSource^ image,
            MediaGraphicsDevice^ device
            );

        ///<summary>Creates a sample presenter with format Bgra8 and resolution SwapChainPanel.Width x SwapChainPanel.Height.</summary>
        static MediaSamplePresenter^ CreateFromSwapChainPanel(
            WUXC::SwapChainPanel^ panel,
            MediaGraphicsDevice^ device,
            int width,
            int height
            );
        
        // IClosable
        virtual ~MediaSamplePresenter();

        void Present(MediaSample^ sample);

    private:

        void _PresentToSurfaceImageSource(const MW::ComPtr<IMFDXGIBuffer>& dxgiBuffer);
        void _PresentToSwapChainPanel(const MW::ComPtr<IMFDXGIBuffer>& dxgiBuffer);

        MW::ComPtr<ISurfaceImageSourceNative> _image;
        MW::ComPtr<ISwapChainPanelNative> _panel;
        MW::ComPtr<IDXGISwapChain1> _swapChain;
    };
}
