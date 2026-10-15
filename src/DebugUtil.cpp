#include "DebugUtil.h"

#include <winrt/base.h>
#include <d3d11.h>
#include <d3dcompiler.h>

namespace DebugRenderer
{
    using DrawFunc = std::function<void()>;

    struct Context
    {
        IDXGISwapChain* swapChain = nullptr;

        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> context;

        float width = 0.f;
        float height = 0.f;
    };

    inline Context g_context;

    inline std::vector<DrawFunc> g_presentCallbacks;

    struct SavedState
    {
        winrt::com_ptr<ID3D11DepthStencilView> depthStencilView;
        winrt::com_ptr<ID3D11RenderTargetView> renderTarget;

        winrt::com_ptr<ID3D11DepthStencilState> depthState;
        UINT stencilRef = 0;

        winrt::com_ptr<ID3D11BlendState> blendState;
        FLOAT blendFactor[4]{};
        UINT sampleMask = 0xffffffff;

        winrt::com_ptr<ID3D11RasterizerState> rasterState;

        D3D11_VIEWPORT viewport{};
    };

    inline REL::Relocation<HRESULT(
        IDXGISwapChain*,
        UINT,
        UINT)> _Present;

    inline void OnPresent(DrawFunc fn)
    {
        g_presentCallbacks.emplace_back(std::move(fn));
    }

    inline bool ReadSwapChain()
    {/*
        struct CreationData
        {
            uintptr_t unk0;
            uintptr_t unk1;
            uintptr_t unk2;

            IDXGISwapChain* swapChain;
        };

        auto data = **reinterpret_cast<CreationData**>(RELOCATION_ID(524730, 411349).address());
*/

        auto renderer = RE::BSGraphics::Renderer::GetSingleton();
        auto data = renderer->GetCurrentRenderWindow();
        auto swapChain = data ? reinterpret_cast<IDXGISwapChain*>(data->swapChain) : nullptr;

        if (!swapChain)
            return false;

        g_context.swapChain = swapChain;

        DXGI_SWAP_CHAIN_DESC desc;

        if (FAILED(
            swapChain->GetDesc(&desc)))
            return false;

        RECT rect;

        GetClientRect(
            desc.OutputWindow,
            &rect);

        g_context.width =
            static_cast<float>(rect.right);

        g_context.height =
            static_cast<float>(rect.bottom);

        return true;
    }

    struct Vertex
    {
        float px,py,pz,pw;
        float r,g,b,a;
        float u,v;
    };

    struct Line
    {
        RE::NiPoint3 start;
        RE::NiPoint3 end;
        RE::NiColorA color;
    };

    struct Point
    {
        RE::NiPoint3 position;
        float radius;
        RE::NiColorA color;
    };

    inline std::vector<Line> g_lines;
    inline std::vector<Point> g_points;

    constexpr char VertexShader[] = R"SHADER(

    struct VS_INPUT
    {
        float4 pos : POS;
        float4 col : COL;
        float2 uv  : TEX;
    };

    struct VS_OUTPUT
    {
        float4 pos : SV_POSITION;
        float4 col : COLOR0;
        float2 uv  : TEXCOORD0;
    };

    VS_OUTPUT main(VS_INPUT input)
    {
        VS_OUTPUT o;

        o.pos = input.pos;
        o.col = input.col;
        o.uv = input.uv;

        return o;
    }

    )SHADER";

    constexpr char PixelShader[] = R"SHADER(

    struct PS_INPUT
    {
        float4 pos : SV_POSITION;
        float4 col : COLOR0;
        float2 uv  : TEXCOORD0;
    };

    float4 main(PS_INPUT input) : SV_Target
    {
        float2 p = input.uv * 2.0 - 1.0;

        if(dot(p,p) > 1.0)
            discard;

        return input.col;
    }

    )SHADER";

    inline winrt::com_ptr<::ID3D11VertexShader> g_vs;
    inline winrt::com_ptr<::ID3D11PixelShader>  g_ps;

    inline winrt::com_ptr<::ID3D11InputLayout> g_layout;
    inline winrt::com_ptr<::ID3D11Buffer> g_vertexBuffer;
    inline winrt::com_ptr<::ID3D11Buffer> g_pointBuffer;

    inline winrt::com_ptr<::ID3D11RasterizerState> g_rasterState;
    inline winrt::com_ptr<::ID3D11BlendState> g_blendState;
    inline winrt::com_ptr<::ID3D11DepthStencilState> g_depthState;

    inline winrt::com_ptr<::ID3DBlob> g_vsBlob;

    inline bool g_initialized = false;

    inline bool CreateShaders()
    {
        winrt::com_ptr<ID3DBlob> errors;

        HRESULT hr =
            D3DCompile(
                VertexShader,
                strlen(VertexShader),
                nullptr,
                nullptr,
                nullptr,
                "main",
                "vs_5_0",
                0,
                0,
                g_vsBlob.put(),
                errors.put());

        if (FAILED(hr))
            return false;

        hr =
            g_context.device->CreateVertexShader(
                g_vsBlob->GetBufferPointer(),
                g_vsBlob->GetBufferSize(),
                nullptr,
                g_vs.put());

        if (FAILED(hr))
            return false;

        winrt::com_ptr<ID3DBlob> psBlob;

        hr =
            D3DCompile(
                PixelShader,
                strlen(PixelShader),
                nullptr,
                nullptr,
                nullptr,
                "main",
                "ps_5_0",
                0,
                0,
                psBlob.put(),
                errors.put());

        if (FAILED(hr))
            return false;

        hr =
            g_context.device->CreatePixelShader(
                psBlob->GetBufferPointer(),
                psBlob->GetBufferSize(),
                nullptr,
                g_ps.put());

        return SUCCEEDED(hr);
    }

    inline bool CreateInputLayout()
    {
        D3D11_INPUT_ELEMENT_DESC desc[]
        {
            {
                "POS",
                0,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                0,
                offsetof(Vertex, px),
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            },

            {
                "COL",
                0,
                DXGI_FORMAT_R32G32B32A32_FLOAT,
                0,
                offsetof(Vertex, r),
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            },

            {
                "TEX",
                0,
                DXGI_FORMAT_R32G32_FLOAT,
                0,
                offsetof(Vertex, u),
                D3D11_INPUT_PER_VERTEX_DATA,
                0
            }
        };

        return SUCCEEDED(
            g_context.device->CreateInputLayout(
                desc,
                3,
                g_vsBlob->GetBufferPointer(),
                g_vsBlob->GetBufferSize(),
                g_layout.put()));
    }

    inline bool CreateVertexBuffer()
    {
        D3D11_BUFFER_DESC bd{};

        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        bd.ByteWidth =
            sizeof(Vertex) * 16384;

        bd.CPUAccessFlags =
            D3D11_CPU_ACCESS_WRITE;

        bd.Usage =
            D3D11_USAGE_DYNAMIC;

        return SUCCEEDED(
            g_context.device->CreateBuffer(
                &bd,
                nullptr,
                g_vertexBuffer.put()));
    }

    inline bool CreatePointBuffer()
    {
        D3D11_BUFFER_DESC bd{};

        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        bd.ByteWidth =
            sizeof(Vertex) * 16384;

        bd.CPUAccessFlags =
            D3D11_CPU_ACCESS_WRITE;

        bd.Usage =
            D3D11_USAGE_DYNAMIC;

        return SUCCEEDED(
            g_context.device->CreateBuffer(
                &bd,
                nullptr,
                g_pointBuffer.put()));
    }

    inline bool CreateRasterizerState()
    {
        D3D11_RASTERIZER_DESC desc{};

        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;

        desc.DepthClipEnable = TRUE;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = TRUE;

        return SUCCEEDED(
            g_context.device->CreateRasterizerState(
                &desc,
                g_rasterState.put()));
    }

    inline bool CreateBlendState()
    {
        D3D11_BLEND_DESC desc{};

        auto& rt = desc.RenderTarget[0];

        rt.BlendEnable = TRUE;

        rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        rt.BlendOp = D3D11_BLEND_OP_ADD;

        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_ZERO;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;

        rt.RenderTargetWriteMask =
            D3D11_COLOR_WRITE_ENABLE_ALL;

        return SUCCEEDED(
            g_context.device->CreateBlendState(
                &desc,
                g_blendState.put()));
    }

    inline bool CreateDepthState()
    {
        D3D11_DEPTH_STENCIL_DESC desc{};

        desc.DepthEnable = TRUE;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = D3D11_COMPARISON_ALWAYS;

        return SUCCEEDED(
            g_context.device->CreateDepthStencilState(
                &desc,
                g_depthState.put()));
    }

    inline bool InitializeRenderer()
    {
        if (g_initialized)
            return true;

        if (!CreateShaders())
            return false;

        if (!CreateInputLayout())
            return false;

        if (!CreateVertexBuffer())
            return false;

        if (!CreatePointBuffer())
            return false;

        if (!CreateRasterizerState())
            return false;

        if (!CreateBlendState())
            return false;

        if (!CreateDepthState())
            return false;

        g_initialized = true;

        return true;
    }

    void DrawLine(
        const RE::NiPoint3& start,
        const RE::NiPoint3& end,
        const RE::NiColorA& color)
    {
        g_lines.push_back(
        {
            start,
            end,
            color
        });
    }

    void DrawPoint(
        const RE::NiPoint3& pos,
        float radius,
        const RE::NiColorA& color)
    {
        g_points.push_back(
        {
            pos,
            radius,
            color
        });
    }

    void DrawPlainAxis(
        const RE::NiPoint3& pos,
        float size,
        const RE::NiColorA& color)
    {
        DrawLine(
            pos + RE::NiPoint3(size,0,0),
            pos + RE::NiPoint3(-size,0,0),
            color);

        DrawLine(
            pos + RE::NiPoint3(0,size,0),
            pos + RE::NiPoint3(0,-size,0),
            color);

        DrawLine(
            pos + RE::NiPoint3(0,0,size),
            pos + RE::NiPoint3(0,0,-size),
            color);
    }

    inline uintptr_t WorldToCam =
        RELOCATION_ID(519579, 406126).address();

    inline RE::NiRect<float>* ViewPort =
        reinterpret_cast<RE::NiRect<float>*>(
            RELOCATION_ID(519618, 406160).address());

    inline bool WriteVertex(
        Vertex& out,
        const RE::NiPoint3& world,
        const RE::NiColorA& color)
    {
        float sx;
        float sy;
        float sz;

        if (!RE::NiCamera::WorldPtToScreenPt3(
            (float(*)[4])WorldToCam,
            *ViewPort,
            world,
            sx,
            sy,
            sz,
            0.0001f))
        {
            return false;
        }

        out.px = (sx - 0.5f) * 2.f;
        out.py = (sy - 0.5f) * 2.f;
        out.pz = sz;
        out.pw = 1.0f;

        out.r = color.red;
        out.g = color.green;
        out.b = color.blue;
        out.a = color.alpha;

        out.u = 0.5f;
        out.v = 0.5f;

        return true;
    }

    inline void FlushLines()
    {
        if (g_lines.empty())
            return;

        D3D11_MAPPED_SUBRESOURCE mapped{};

        if (FAILED(
            g_context.context->Map(
                g_vertexBuffer.get(),
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mapped)))
        {
            return;
        }

        Vertex* out =
            reinterpret_cast<Vertex*>(mapped.pData);

        uint32_t vertexCount = 0;

        for (auto& line : g_lines)
        {
            constexpr uint32_t MaxVertices = 16384;

            if (vertexCount + 6 > MaxVertices)
                break;

            if (!WriteVertex(out[0], line.start, line.color))
                continue;

            if (!WriteVertex(out[1], line.end, line.color))
                continue;

            out += 2;
            vertexCount += 2;
        }

        g_context.context->Unmap(
            g_vertexBuffer.get(),
            0);

        if (!vertexCount)
        {
            g_lines.clear();
            return;
        }

        UINT stride = sizeof(Vertex);
        UINT offset = 0;

        ::ID3D11Buffer* vb = g_vertexBuffer.get();

        g_context.context->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

        g_context.context->IASetInputLayout(
            g_layout.get());

        g_context.context->IASetVertexBuffers(
            0,
            1,
            &vb,
            &stride,
            &offset);

        g_context.context->RSSetState(
            g_rasterState.get());

        float factors[4] =
        {
            1.f,
            1.f,
            1.f,
            1.f
        };

        g_context.context->OMSetBlendState(
            g_blendState.get(),
            factors,
            0xffffffff);

        g_context.context->OMSetDepthStencilState(
            g_depthState.get(),
            0);

        g_context.context->VSSetShader(
            g_vs.get(),
            nullptr,
            0);

        g_context.context->PSSetShader(
            g_ps.get(),
            nullptr,
            0);

        D3D11_VIEWPORT vp{};

        vp.TopLeftX = 0;
        vp.TopLeftY = 0;

        vp.Width = g_context.width;
        vp.Height = g_context.height;

        vp.MinDepth = 0;
        vp.MaxDepth = 1;

        g_context.context->RSSetViewports(
            1,
            &vp);

        g_context.context->Draw(
            vertexCount,
            0);

        g_lines.clear();
    }

    inline void FlushPoints()
    {
        if (g_points.empty())
            return;

        D3D11_MAPPED_SUBRESOURCE mapped{};

        if (FAILED(
            g_context.context->Map(
                g_pointBuffer.get(),
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mapped)))
        {
            return;
        }

        auto* out =
            reinterpret_cast<Vertex*>(mapped.pData);

        uint32_t vertexCount = 0;

        for (auto& point : g_points)
        {
            constexpr uint32_t MaxVertices = 16384;

            if (vertexCount + 6 > MaxVertices)
                break;

            float sx;
            float sy;
            float sz;

            if (!RE::NiCamera::WorldPtToScreenPt3(
                (float(*)[4])WorldToCam,
                *ViewPort,
                point.position,
                sx,
                sy,
                sz,
                0.0001f))
            {
                continue;
            }

            float radiusX = point.radius / g_context.width * 2.f;
            float radiusY = point.radius / g_context.height * 2.f;

            float x = (sx - 0.5f) * 2.f;
            float y = (sy - 0.5f) * 2.f;

            Vertex v[6];

            auto Fill =
            [&](Vertex& vert,
                float px,
                float py,
                float u,
                float vcoord)
            {
                vert.px = px;
                vert.py = py;
                vert.pz = sz;
                vert.pw = 1.f;

                vert.r = point.color.red;
                vert.g = point.color.green;
                vert.b = point.color.blue;
                vert.a = point.color.alpha;

                vert.u = u;
                vert.v = vcoord;
            };

            Fill(v[0], x-radiusX, y-radiusY, 0,1);
            Fill(v[1], x+radiusX, y-radiusY, 1,1);
            Fill(v[2], x-radiusX, y+radiusY, 0,0);

            Fill(v[3], x-radiusX, y+radiusY, 0,0);
            Fill(v[4], x+radiusX, y-radiusY, 1,1);
            Fill(v[5], x+radiusX, y+radiusY, 1,0);

            memcpy(out, v, sizeof(v));

            out += 6;
            vertexCount += 6;
        }

        g_context.context->Unmap(
            g_pointBuffer.get(),
            0);

        if (!vertexCount)
        {
            g_points.clear();
            return;
        }

        UINT stride = sizeof(Vertex);
        UINT offset = 0;

        ::ID3D11Buffer* vb = g_pointBuffer.get();

        g_context.context->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        g_context.context->IASetInputLayout(
            g_layout.get());

        g_context.context->IASetVertexBuffers(
            0,
            1,
            &vb,
            &stride,
            &offset);

        float factors[4] = { 1,1,1,1 };

        g_context.context->RSSetState(
            g_rasterState.get());

        g_context.context->OMSetBlendState(
            g_blendState.get(),
            factors,
            0xffffffff);

        g_context.context->OMSetDepthStencilState(
            g_depthState.get(),
            0);

        g_context.context->VSSetShader(
            g_vs.get(),
            nullptr,
            0);

        g_context.context->PSSetShader(
            g_ps.get(),
            nullptr,
            0);

        D3D11_VIEWPORT vp{};
        vp.Width = g_context.width;
        vp.Height = g_context.height;
        vp.MinDepth = 0.f;
        vp.MaxDepth = 1.f;

        g_context.context->RSSetViewports(
            1,
            &vp);

        g_context.context->Draw(
            vertexCount,
            0);

        g_points.clear();
    }

    inline HRESULT Present(
        IDXGISwapChain* swapChain,
        UINT sync,
        UINT flags)
    {
        SavedState state;

        auto& ctx = g_context.context;

        ctx->OMGetDepthStencilState(
            state.depthState.put(),
            &state.stencilRef);

        ctx->OMGetBlendState(
            state.blendState.put(),
            state.blendFactor,
            &state.sampleMask);

        UINT vp = 1;

        ctx->RSGetViewports(
            &vp,
            &state.viewport);

        ctx->RSGetState(
            state.rasterState.put());

        ctx->OMGetRenderTargets(
            1,
            state.renderTarget.put(),
            state.depthStencilView.put());

        if (!g_initialized)
        {
            if (!InitializeRenderer())
                return _Present(swapChain, sync, flags);
        }

        for (auto& fn : g_presentCallbacks)
            fn();

        FlushLines();
        FlushPoints();

        auto rt = state.renderTarget.get();

        ctx->OMSetRenderTargets(
            1,
            &rt,
            state.depthStencilView.get());

        ctx->RSSetState(
            state.rasterState.get());

        ctx->RSSetViewports(
            1,
            &state.viewport);

        ctx->OMSetBlendState(
            state.blendState.get(),
            state.blendFactor,
            state.sampleMask);

        ctx->OMSetDepthStencilState(
            state.depthState.get(),
            state.stencilRef);

        return _Present(
            swapChain,
            sync,
            flags);
    }

    bool Install()
    {
        if (!ReadSwapChain())
            return false;

        if (FAILED(
            g_context.swapChain->GetDevice(
                __uuidof(ID3D11Device),
                g_context.device.put_void())))
            return false;

        g_context.device->GetImmediateContext(
            g_context.context.put());

        REL::Relocation<std::uintptr_t> vtbl{
            *(uintptr_t*)g_context.swapChain
        };

        _Present =
            vtbl.write_vfunc(
                8,
                Present);

        return true;
    }

}
