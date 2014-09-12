//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::Foundation::Collections;

namespace angle_test_surfacehost
{
    TEST_CLASS(SurfaceHostTests)
    {
    private:
        void createPropertyMap(IMap<HSTRING, IInspectable*>** propertyMap)
        {
            ComPtr<IPropertySet> propertySet;
            ComPtr<IActivationFactory> propertySetFactory;
            Assert::AreEqual(S_OK, GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_Collections_PropertySet).Get(), &propertySetFactory));
            Assert::AreEqual(S_OK, propertySetFactory->ActivateInstance(&propertySet));
            Assert::AreEqual(S_OK, propertySet.CopyTo(propertyMap));
        }

        void setBoolPropertyValue(const ComPtr<IMap<HSTRING, IInspectable*>>& propertyMap, const wchar_t* propertyName, bool boolValue)
        {
            ComPtr<IPropertyValueStatics> propertyValueStatics;
            ComPtr<IPropertyValue> propertyValue;
            boolean propertyReplaced = false;

            // Create a new boolean IPropertyValue
            Assert::AreEqual(S_OK, GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get(), propertyValueStatics.GetAddressOf()));
            Assert::AreEqual(S_OK, propertyValueStatics->CreateBoolean(boolValue, &propertyValue));

            // Insert into property map
            Assert::AreEqual(S_OK, propertyMap->Insert(HStringReference(propertyName).Get(), propertyValue.Get(), &propertyReplaced));
        }

        void setSizePropertyValue(const ComPtr<IMap<HSTRING, IInspectable*>>& propertyMap, const wchar_t* propertyName, const Size& size)
        {
            ComPtr<IPropertyValueStatics> propertyValueStatics;
            ComPtr<IPropertyValue> propertyValue;
            boolean propertyReplaced = false;

            // Create a new Size IPropertyValue
            Assert::AreEqual(S_OK, GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get(), propertyValueStatics.GetAddressOf()));
            Assert::AreEqual(S_OK, propertyValueStatics->CreateSize(size, &propertyValue));

            // Insert into property map
            Assert::AreEqual(S_OK, propertyMap->Insert(HStringReference(propertyName).Get(), propertyValue.Get(), &propertyReplaced));
        }

        void setInspectablePropertyValue(const ComPtr<IMap<HSTRING, IInspectable*>>& propertyMap, const wchar_t* propertyName, IInspectable* inspectable)
        {
            ComPtr<IPropertyValueStatics> propertyValueStatics;
            ComPtr<IPropertyValue> propertyValue;
            boolean propertyReplaced = false;

            // Insert into property map
            Assert::AreEqual(S_OK, propertyMap->Insert(HStringReference(propertyName).Get(), inspectable, &propertyReplaced));
        }

    public:
        TEST_METHOD(InitializingSurfaceHostWithNullNativeWindowTypeFails)
        {
            rx::SurfaceHost host(nullptr);
            Assert::IsFalse(host.initialize());
        }
        TEST_METHOD(InitializingSurfaceHostWithInvalidIInspectableFails)
        {
            ComPtr<IAmInspectable> notCoreWindow = Make<GenericIInspectable>();
            rx::SurfaceHost host(notCoreWindow.Get());
            Assert::IsFalse(host.initialize());
        }
        TEST_METHOD(InitializingSurfaceHostWithNonIInspectableFails)
        {
            ComPtr<IAmUnknown> notIInspectable = Make<GenericIUnknown>();
            rx::SurfaceHost host((IInspectable*)notIInspectable.Get());
            Assert::IsFalse(host.initialize());
        }
        TEST_METHOD(InitializingSurfaceHostWithValidCoreWindowSucceeds)
        {
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow = Make<MockCoreWindow>();
            rx::SurfaceHost host(coreWindow.Get());
            Assert::IsTrue(host.initialize());
        }
        TEST_METHOD(InitializingSurfaceHostWithValidSwapChainPanelSucceeds)
        {
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            rx::SurfaceHost host(swapChainPanel.Get());
            Assert::IsTrue(host.initialize());
        }
        TEST_METHOD(InitializingSurfaceHostWithValidPropertySetAndSwapChainPanelSucceeds)
        {
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            createPropertyMap(&propertySet);
            setInspectablePropertyValue(propertySet.Get(), EGLNativeWindowTypeProperty, swapChainPanel.Get());
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsTrue(host.initialize());
        }
        TEST_METHOD(InitializingSurfaceHostWithValidPropertySetAndCoreWindowSucceeds)
        {
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow = Make<MockCoreWindow>();
            createPropertyMap(&propertySet);
            setInspectablePropertyValue(propertySet.Get(), EGLNativeWindowTypeProperty, coreWindow.Get());
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsTrue(host.initialize());
        }
        TEST_METHOD(CoreWindowWithCustomSwapChainSizeMatchesClientRect)
        {
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow = Make<MockCoreWindow>();
            createPropertyMap(&propertySet);
            setInspectablePropertyValue(propertySet.Get(), EGLNativeWindowTypeProperty, coreWindow.Get());
            setSizePropertyValue(propertySet.Get(), EGLRenderSurfaceSizeProperty, { 640, 480 });
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsTrue(host.initialize());
            RECT expectedClientRect = {0, 0, 640, 480 };
            RECT actualClientRect = { 0, 0, 0, 0 };
            host.getClientRect(&actualClientRect);
            Assert::IsTrue(actualClientRect == expectedClientRect);
        }
        TEST_METHOD(CoreWindowWithInvalidCustomSwapChainSizeFails)
        {
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow = Make<MockCoreWindow>();
            createPropertyMap(&propertySet);
            setInspectablePropertyValue(propertySet.Get(), EGLNativeWindowTypeProperty, coreWindow.Get());
            setSizePropertyValue(propertySet.Get(), EGLRenderSurfaceSizeProperty, { 0, 480 });
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsFalse(host.initialize());
        }
        TEST_METHOD(SwapChainPanelWithCustomSwapChainSizeMatchesClientRect)
        {
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            createPropertyMap(&propertySet);
            setInspectablePropertyValue(propertySet.Get(), EGLNativeWindowTypeProperty, swapChainPanel.Get());
            setSizePropertyValue(propertySet.Get(), EGLRenderSurfaceSizeProperty, { 480, 800 });
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsTrue(host.initialize());
            RECT expectedClientRect = { 0, 0, 480, 800 };
            RECT actualClientRect = { 0, 0, 0, 0 };
            host.getClientRect(&actualClientRect);
            Assert::IsTrue(actualClientRect == expectedClientRect);
        }
        TEST_METHOD(InitializingSurfaceHostWithValidPropertySetAndInvalidIInspectableFails)
        {
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            ComPtr<IAmInspectable> notCoreWindow = Make<GenericIInspectable>();
            createPropertyMap(&propertySet);
            setInspectablePropertyValue(propertySet.Get(), EGLNativeWindowTypeProperty, notCoreWindow.Get());
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsFalse(host.initialize());
        }
        TEST_METHOD(InitializingSurfaceHostWithValidPropertySetAndInvalidEGLNativeWindowTypeFails)
        {
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            createPropertyMap(&propertySet);
            setBoolPropertyValue(propertySet.Get(), EGLNativeWindowTypeProperty, false);
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsFalse(host.initialize());
        }
        TEST_METHOD(InitializingSurfaceHostWithValidPropertySetAndMissingEGLNativeWindowTypeFails)
        {
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            createPropertyMap(&propertySet);
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsFalse(host.initialize());
        }
        TEST_METHOD(CoreWindowSurfaceHostCreateSwapChainWithInvalidParamsFailsWithInvalidArg)
        {
            ComPtr<DXGISwapChain> swapChain;
            ComPtr<DXGIFactory> factory = Make<MockDXGIFactory>();
            ComPtr<ID3D11Device> device = Make<MockD3DDevice>();
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow = Make<MockCoreWindow>();
            rx::SurfaceHost host(coreWindow.Get());
            Assert::IsTrue(host.initialize());
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(nullptr, factory.Get(), DXGI_FORMAT_UNKNOWN, 1, 1, swapChain.GetAddressOf()), L"Unexpected success using null device");
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(device.Get(), nullptr, DXGI_FORMAT_UNKNOWN, 1, 1, swapChain.GetAddressOf()), L"Unexpected success using null factory");
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 0, 1, swapChain.GetAddressOf()), L"Unexpected success using 0 width");
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 1, 0, swapChain.GetAddressOf()), L"Unexpected success using 0 height");
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 1, 1, nullptr), L"Unexpected success using null swapchain");
        }

        TEST_METHOD(SwapChainPanelSurfaceHostCreateSwapChainWithInvalidParamsFailsWithInvalidArg)
        {
            ComPtr<DXGISwapChain> swapChain;
            ComPtr<DXGIFactory> factory = Make<MockDXGIFactory>();
            ComPtr<ID3D11Device> device = Make<MockD3DDevice>();
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            rx::SurfaceHost host(swapChainPanel.Get());
            Assert::IsTrue(host.initialize());
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(nullptr, factory.Get(), DXGI_FORMAT_UNKNOWN, 1, 1, swapChain.GetAddressOf()), L"Unexpected success using null device");
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(device.Get(), nullptr, DXGI_FORMAT_UNKNOWN, 1, 1, swapChain.GetAddressOf()), L"Unexpected success using null factory");
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 0, 1, swapChain.GetAddressOf()), L"Unexpected success using 0 width ");
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 1, 0, swapChain.GetAddressOf()), L"Unexpected success using 0 height ");
            Assert::AreEqual(E_INVALIDARG, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 1, 1, nullptr), L"Unexpected success using null swapchain");
        }

        TEST_METHOD(SwapChainPanelSurfaceHostCreateSwapChainUsesValidDesc)
        {
            DXGI_SWAP_CHAIN_DESC1 expectedDesc = 
            {
                1, 1,                               // Width/Height
                DXGI_FORMAT_UNKNOWN,                // DXGI_FORMAT
                FALSE,                              // Stereo
                { 1, 0 },                           // DXGI_SAMPLE_DESC
                DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER,    // DXGI_USAGE
                2,                                  // BufferCount
                DXGI_SCALING_STRETCH,               // DXGI_SCALING
                DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,   // DXGI_SWAP_EFFECT
                DXGI_ALPHA_MODE_IGNORE,             // DXGI_ALPHA_MODE
                0                                   // Flags
            };
            DXGI_SWAP_CHAIN_DESC1 desc;

            ComPtr<DXGISwapChain> swapChain;
            ComPtr<DXGIFactory> factory = Make<MockDXGIFactory>();
            ComPtr<ID3D11Device> device = Make<MockD3DDevice>();
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            rx::SurfaceHost host(swapChainPanel.Get());
            Assert::IsTrue(host.initialize());
            Assert::AreEqual(S_OK, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 1, 1, swapChain.GetAddressOf()));
            Assert::AreEqual(S_OK, swapChain->GetDesc1(&desc));
            Assert::AreEqual(expectedDesc.Width, desc.Width);
            Assert::AreEqual(expectedDesc.Height, desc.Height);
            Assert::AreEqual(expectedDesc.Stereo, desc.Stereo);
            Assert::AreEqual(expectedDesc.SampleDesc.Count, desc.SampleDesc.Count);
            Assert::AreEqual(expectedDesc.SampleDesc.Quality, desc.SampleDesc.Quality);
            Assert::AreEqual(expectedDesc.BufferUsage, desc.BufferUsage);
            Assert::AreEqual(expectedDesc.BufferCount, desc.BufferCount);
            Assert::AreEqual(expectedDesc.Flags, desc.Flags);
            // Compare enum values.  
            // Assert::AreEqual does not understand DirectX enum values and requires a custom ToString implementation.
            Assert::IsTrue(expectedDesc.Format == desc.Format, L"Unexpected DXGI_FORMAT");
            Assert::IsTrue(expectedDesc.Scaling == desc.Scaling, L"Unexpected DXGI_SCALING");
            Assert::IsTrue(expectedDesc.SwapEffect == desc.SwapEffect, L"Unexpected DXGI_SWAP_EFFECT");
            Assert::IsTrue(expectedDesc.AlphaMode == desc.AlphaMode, L"Unexpected DXGI_ALPHA_MODE");
        }

        TEST_METHOD(CoreWindowSurfaceHostCreateSwapChainUsesValidDesc)
        {
            DXGI_SWAP_CHAIN_DESC1 expectedDesc =
            {
                1, 1,                               // Width/Height
                DXGI_FORMAT_UNKNOWN,                // DXGI_FORMAT
                FALSE,                              // Stereo
                { 1, 0 },                           // DXGI_SAMPLE_DESC
                DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER,    // DXGI_USAGE
                2,                                  // BufferCount
                DXGI_SCALING_STRETCH,               // DXGI_SCALING
                DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,   // DXGI_SWAP_EFFECT
                DXGI_ALPHA_MODE_UNSPECIFIED,        // DXGI_ALPHA_MODE
                0                                   // Flags
            };
            DXGI_SWAP_CHAIN_DESC1 desc;

            ComPtr<DXGISwapChain> swapChain;
            ComPtr<DXGIFactory> factory = Make<MockDXGIFactory>();
            ComPtr<ID3D11Device> device = Make<MockD3DDevice>();
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow = Make<MockCoreWindow>();
            rx::SurfaceHost host(coreWindow.Get());
            Assert::IsTrue(host.initialize());
            Assert::AreEqual(S_OK, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 1, 1, swapChain.GetAddressOf()));
            Assert::AreEqual(S_OK, swapChain->GetDesc1(&desc));
            Assert::AreEqual(expectedDesc.Width, desc.Width);
            Assert::AreEqual(expectedDesc.Height, desc.Height);
            Assert::AreEqual(expectedDesc.Stereo, desc.Stereo);
            Assert::AreEqual(expectedDesc.SampleDesc.Count, desc.SampleDesc.Count);
            Assert::AreEqual(expectedDesc.SampleDesc.Quality, desc.SampleDesc.Quality);
            Assert::AreEqual(expectedDesc.BufferUsage, desc.BufferUsage);
            Assert::AreEqual(expectedDesc.BufferCount, desc.BufferCount);
            Assert::AreEqual(expectedDesc.Flags, desc.Flags);
            // Compare enum values.  
            // Assert::AreEqual does not understand DirectX enum values and requires a custom ToString implementation.
            Assert::IsTrue(expectedDesc.Format == desc.Format, L"Unexpected DXGI_FORMAT");
            Assert::IsTrue(expectedDesc.Scaling == desc.Scaling, L"Unexpected DXGI_SCALING");
            Assert::IsTrue(expectedDesc.SwapEffect == desc.SwapEffect, L"Unexpected DXGI_SWAP_EFFECT");
            Assert::IsTrue(expectedDesc.AlphaMode == desc.AlphaMode, L"Unexpected DXGI_ALPHA_MODE");
        }

        TEST_METHOD(CoreWindowSizeChangedEventIsUnregisteredOnDestruction)
        {
            ComPtr<MockCoreWindow> coreWindow = Make<MockCoreWindow>();
            {
                rx::SurfaceHost host(coreWindow.Get());
                Assert::IsTrue(host.initialize());
                Assert::AreEqual(1, coreWindow->GetSizeChangeRegistrationCount());
            }
            Assert::AreEqual(0, coreWindow->GetSizeChangeRegistrationCount());
        }
        TEST_METHOD(SwapChainPanelSizeChangedEventIsUnregisteredOnDestruction)
        {
            ComPtr<MockSwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            {
                ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> panel;
                Assert::AreEqual(S_OK, swapChainPanel.As(&panel));
                Assert::IsNotNull(panel.Get());
                rx::SurfaceHost host(panel.Get());
                Assert::IsTrue(host.initialize());
                Assert::AreEqual(1, swapChainPanel->GetSizeChangeRegistrationCount());
            }
            Assert::AreEqual(0, swapChainPanel->GetSizeChangeRegistrationCount());
        }

        // If this test is run on Windows Phone devices, an expected LogicalDpi
        // value of 96.0f will be used because IDisplayPropertiesStatics::get_LogicalDpi
        // fails with HRESULT_FROM_WIN32(ERROR_NOT_FOUND).
        TEST_METHOD(CoreWindowCurrentBoundsIsQueriedOnInitialization)
        {
            ComPtr<MockCoreWindow> coreWindow = Make<MockCoreWindow>();
            ABI::Windows::Foundation::Rect expectedBounds = { 0, 0, 1024, 768 };
            coreWindow->SetExpectedBounds(expectedBounds);

            rx::SurfaceHost host(coreWindow.Get());
            Assert::IsTrue(host.initialize());
            Assert::AreEqual(1, coreWindow->GetBoundsQueryCount());

            RECT clientRect = { 0, 0, 0, 0 };
            RECT expectedRect = { 0, 0, 1024, 768 };
            Assert::IsTrue(host.getClientRect(&clientRect));
            Assert::IsTrue(clientRect == expectedRect);

            Assert::AreEqual(1, coreWindow->GetBoundsQueryCount());
        }
        TEST_METHOD(SwapChainPanelCurrentBoundsIsQueriedOnInitialization)
        {
            ComPtr<MockSwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            ABI::Windows::Foundation::Rect expectedBounds = { 0, 0, 1024, 768 };
            swapChainPanel->SetExpectedBounds(expectedBounds);

            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> panel;
            Assert::AreEqual(S_OK, swapChainPanel.As(&panel));
            Assert::IsNotNull(panel.Get());
            rx::SurfaceHost host(panel.Get());
            Assert::IsTrue(host.initialize());
            Assert::AreEqual(1, swapChainPanel->GetBoundsQueryCount());

            RECT clientRect = { 0, 0, 0, 0 };
            RECT expectedRect = { 0, 0, 1024, 768 };
            Assert::IsTrue(host.getClientRect(&clientRect));
            Assert::IsTrue(clientRect == expectedRect);

            Assert::AreEqual(1, swapChainPanel->GetBoundsQueryCount());
        }
        TEST_METHOD(CoreWindowSurfaceIsIconicAlwaysReturnsFalse)
        {
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow = Make<MockCoreWindow>();
            rx::SurfaceHost host(coreWindow.Get());
            Assert::IsTrue(host.initialize());
            Assert::IsFalse(host.isIconic());
        }
        TEST_METHOD(SwapChainPanelSurfaceIsIconicAlwaysReturnsFalse)
        {
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            rx::SurfaceHost host(swapChainPanel.Get());
            Assert::IsTrue(host.initialize());
            Assert::IsFalse(host.isIconic());
        }

        // If this test is run on Windows Phone devices, an expected LogicalDpi
        // value of 96.0f will be used because IDisplayPropertiesStatics::get_LogicalDpi
        // fails with HRESULT_FROM_WIN32(ERROR_NOT_FOUND).
        TEST_METHOD(CoreWindowSizeChangedEventSignalUpdatesClientRectToExpectedValue)
        {
            // Create core window with 800x600 bounds
            ComPtr<MockCoreWindow> coreWindow = Make<MockCoreWindow>();
            ABI::Windows::Foundation::Rect expectedBounds = { 0, 0, 800, 600 };
            coreWindow->SetExpectedBounds(expectedBounds);

            // Create surface, expecting to see 800x600 bounds
            rx::SurfaceHost host(coreWindow.Get());
            Assert::IsTrue(host.initialize());

            RECT clientRect = { 0, 0, 0, 0 };
            RECT expectedRect = { 0, 0, 800, 600 };
            Assert::IsTrue(host.getClientRect(&clientRect));
            Assert::IsTrue(clientRect == expectedRect);

            // Update core window to 1024x768 bounds, and cause a sizeChangedEvent to be signaled
            clientRect = { 0, 0, 0, 0 };
            expectedBounds = { 0, 0, 1024, 768 };
            coreWindow->UpdateSizeAndSignalChangedEvent(expectedBounds);

            expectedRect = { 0, 0, 1024, 768 };
            Assert::IsTrue(host.getClientRect(&clientRect));
            Assert::IsTrue(clientRect == expectedRect);
        }

        TEST_METHOD(SwapChainPanelSizeChangedEventSignalUpdatesClientRectToExpectedValue)
        {
            // Create swapchainpanel with 800x600 bounds
            ComPtr<MockSwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            ABI::Windows::Foundation::Rect expectedBounds = { 0, 0, 800, 600 };
            swapChainPanel->SetExpectedBounds(expectedBounds);

            // Create surface, expecting to see 800x600 bounds
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> panel;
            Assert::AreEqual(S_OK, swapChainPanel.As(&panel));
            Assert::IsNotNull(panel.Get());
            rx::SurfaceHost host(panel.Get());
            Assert::IsTrue(host.initialize());

            RECT clientRect = { 0, 0, 0, 0 };
            RECT expectedRect = { 0, 0, 800, 600 };
            Assert::IsTrue(host.getClientRect(&clientRect));
            Assert::IsTrue(clientRect == expectedRect);

            // Update core window to 1024x768 bounds, and cause a sizeChangedEvent to be signaled
            clientRect = { 0, 0, 0, 0 };
            expectedBounds = { 0, 0, 1024, 768 };
            swapChainPanel->UpdateSizeAndSignalChangedEvent(expectedBounds);

            expectedRect = { 0, 0, 1024, 768 };
            Assert::IsTrue(host.getClientRect(&clientRect));
            Assert::IsTrue(clientRect == expectedRect);
        }

        TEST_METHOD(SwapChainPanelWithLargeCustomSwapChainSizeConfiguresValidScaleTransform)
        {
            // Create swapchainpanel with 800x600 bounds
            ComPtr<MockSwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            ABI::Windows::Foundation::Rect expectedBounds = { 0, 0, 800, 600 };
            swapChainPanel->SetExpectedBounds(expectedBounds);

            // Create surface, expecting to see 1024x768 pixel bounds
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> panel;
            Assert::AreEqual(S_OK, swapChainPanel.As(&panel));
            Assert::IsNotNull(panel.Get());
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            createPropertyMap(&propertySet);
            setInspectablePropertyValue(propertySet.Get(), EGLNativeWindowTypeProperty, panel.Get());
            setSizePropertyValue(propertySet.Get(), EGLRenderSurfaceSizeProperty, { 1024, 768 });
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsTrue(host.initialize());
            RECT expectedClientRect = { 0, 0, 1024, 768 };
            RECT actualClientRect = { 0, 0, 0, 0 };
            host.getClientRect(&actualClientRect);
            Assert::IsTrue(actualClientRect == expectedClientRect);

            // Create a 1024x768 swapchain and validate that the Transform set on the swapchain
            // is properly configured for scaling up to match full 800x600 renderable area.
            ComPtr<DXGISwapChain> swapChain;
            ComPtr<DXGIFactory> factory = Make<MockDXGIFactory>();
            ComPtr<ID3D11Device> device = Make<MockD3DDevice>();
            Assert::AreEqual(S_OK, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 1024, 768, swapChain.GetAddressOf()));

            // Calculate expected scale transform which should be 2X
            // actual size.
            DXGI_MATRIX_3X2_F actualScaleTransform = { 0 };
            DXGI_MATRIX_3X2_F expectedScaleTransform = { 0 };
            ABI::Windows::Foundation::Size renderScale = { 800.0f / 1024.0f, 600.0f / 768.0f };
            expectedScaleTransform._11 = renderScale.Width;
            expectedScaleTransform._22 = renderScale.Height;

            ComPtr<IDXGISwapChain2> swapChain2;
            Assert::AreEqual(S_OK, swapChain.As(&swapChain2));
            Assert::AreEqual(S_OK, swapChain2->GetMatrixTransform(&actualScaleTransform));

            Assert::AreEqual(expectedScaleTransform._11, actualScaleTransform._11, L"Invalid matrix field _11");
            Assert::AreEqual(expectedScaleTransform._12, actualScaleTransform._12, L"Invalid matrix field _12");
            Assert::AreEqual(expectedScaleTransform._21, actualScaleTransform._21, L"Invalid matrix field _21");
            Assert::AreEqual(expectedScaleTransform._22, actualScaleTransform._22, L"Invalid matrix field _22");
            Assert::AreEqual(expectedScaleTransform._31, actualScaleTransform._31, L"Invalid matrix field _31");
            Assert::AreEqual(expectedScaleTransform._32, actualScaleTransform._32, L"Invalid matrix field _32");

        }

        TEST_METHOD(SwapChainPanelWithSmallCustomSwapChainSizeConfiguresValidScaleTransform)
        {
            // Create swapchainpanel with 800x600 bounds
            ComPtr<MockSwapChainPanel> swapChainPanel = Make<MockSwapChainPanel>();
            ABI::Windows::Foundation::Rect expectedBounds = { 0, 0, 800, 600 };
            swapChainPanel->SetExpectedBounds(expectedBounds);

            // Create surface, expecting to see 400x300 pixel bounds
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> panel;
            Assert::AreEqual(S_OK, swapChainPanel.As(&panel));
            Assert::IsNotNull(panel.Get());
            ComPtr<IMap<HSTRING, IInspectable*>> propertySet;
            createPropertyMap(&propertySet);
            setInspectablePropertyValue(propertySet.Get(), EGLNativeWindowTypeProperty, panel.Get());
            setSizePropertyValue(propertySet.Get(), EGLRenderSurfaceSizeProperty, { 400, 300 });
            rx::SurfaceHost host(propertySet.Get());
            Assert::IsTrue(host.initialize());
            RECT expectedClientRect = { 0, 0, 400, 300 };
            RECT actualClientRect = { 0, 0, 0, 0 };
            host.getClientRect(&actualClientRect);
            Assert::IsTrue(actualClientRect == expectedClientRect);

            // Create a 400x300 swapchain and validate that the Transform set on the swapchain
            // is properly configured for scaling up to match full 800x600 renderable area.
            ComPtr<DXGISwapChain> swapChain;
            ComPtr<DXGIFactory> factory = Make<MockDXGIFactory>();
            ComPtr<ID3D11Device> device = Make<MockD3DDevice>();
            Assert::AreEqual(S_OK, host.createSwapChain(device.Get(), factory.Get(), DXGI_FORMAT_UNKNOWN, 400, 300, swapChain.GetAddressOf()));

            // Calculate expected scale transform
            DXGI_MATRIX_3X2_F actualScaleTransform = { 0 };
            DXGI_MATRIX_3X2_F expectedScaleTransform = { 0 };
            ABI::Windows::Foundation::Size renderScale = { 800.0f / 400.0f, 600.0f / 300.0f };
            expectedScaleTransform._11 = renderScale.Width;
            expectedScaleTransform._22 = renderScale.Height;

            ComPtr<IDXGISwapChain2> swapChain2;
            Assert::AreEqual(S_OK, swapChain.As(&swapChain2));
            Assert::AreEqual(S_OK, swapChain2->GetMatrixTransform(&actualScaleTransform));

            Assert::AreEqual(expectedScaleTransform._11, actualScaleTransform._11, L"Invalid matrix field _11");
            Assert::AreEqual(expectedScaleTransform._12, actualScaleTransform._12, L"Invalid matrix field _12");
            Assert::AreEqual(expectedScaleTransform._21, actualScaleTransform._21, L"Invalid matrix field _21");
            Assert::AreEqual(expectedScaleTransform._22, actualScaleTransform._22, L"Invalid matrix field _22");
            Assert::AreEqual(expectedScaleTransform._31, actualScaleTransform._31, L"Invalid matrix field _31");
            Assert::AreEqual(expectedScaleTransform._32, actualScaleTransform._32, L"Invalid matrix field _32");
        }

    };
}