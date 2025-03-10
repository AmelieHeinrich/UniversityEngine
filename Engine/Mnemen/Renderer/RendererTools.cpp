//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-04 10:38:05
//

#include "RendererTools.hpp"

#include <RHI/Uploader.hpp>

RendererTools::RendererToolsData RendererTools::sData;

View::Ref RenderPassResource::AddView(ViewType type, ViewDimension dimension, TextureFormat format)
{
    View::Ref view = ParentRHI->CreateView(Texture, type, dimension, format);
    Views.push_back(view);
    return view;
}

View::Ref RenderPassResource::GetView(ViewType type)
{
    for (auto& view : Views) {
        if (view->GetType() == type) {
            return view;
        }
    }
    return nullptr;
}

int RenderPassResource::Descriptor(ViewType type, int frameIndex)
{
    switch (Type) {
        case RenderPassResourceType::SharedTexture: {
            return GetView(type)->GetDescriptor().Index;
        }
        case RenderPassResourceType::SharedBuffer: {
            switch (type) {
                case ViewType::Storage: {
                    return Buffer->UAV();
                }
                case ViewType::ShaderResource: {
                    return Buffer->SRV();
                }
            }
            break;
        }
        case RenderPassResourceType::SharedSampler: {
            return Sampler->BindlesssSampler();
            break;
        }
        case RenderPassResourceType::SharedRingBuffer: {
            switch (type) {
                case ViewType::ShaderResource: {
                    return RBuffer[frameIndex]->SRV();
                }
                case ViewType::None: {
                    return RBuffer[frameIndex]->CBV();;
                }
            }
        }
    }
    return -1;
}

void RendererTools::Init(RHI::Ref rhi)
{
    sData.RHI = rhi;

    {
        TextureDesc desc = {};
        desc.Width = 1;
        desc.Height = 1;
        desc.Levels = 1;
        desc.Depth = 1;
        desc.Name = "White Texture";
        desc.Format = TextureFormat::RGBA8;
        
        auto tex = CreateSharedTexture("WhiteTexture", desc);
        tex->AddView(ViewType::ShaderResource);
        tex->Texture->Tag(ResourceTag::RenderPassIO);

        Uploader::EnqueueTextureUpload(Vector<UInt8>{ 0xFF, 0xFF, 0xFF, 0xFF }, tex->Texture);
    }
    {
        TextureDesc desc = {};
        desc.Width = 1;
        desc.Height = 1;
        desc.Levels = 1;
        desc.Depth = 1;
        desc.Name = "Black Texture";
        desc.Format = TextureFormat::RGBA8;
        
        auto tex = CreateSharedTexture("BlackTexture", desc);
        tex->AddView(ViewType::ShaderResource);
        tex->Texture->Tag(ResourceTag::RenderPassIO);
        
        Uploader::EnqueueTextureUpload(Vector<UInt8>{ 0x00, 0x00, 0x00, 0xFF }, tex->Texture);
    }
}

Ref<RenderPassResource> RendererTools::CreateSharedTexture(const String& name, TextureDesc desc)
{
    Ref<RenderPassResource> resource = MakeRef<RenderPassResource>();
    resource->ParentRHI = sData.RHI;
    resource->Type = RenderPassResourceType::SharedTexture;
    resource->Texture = sData.RHI->CreateTexture(desc);
    resource->Texture->Tag(ResourceTag::RenderPassIO);
    sData.Resources[name] = resource;
    return sData.Resources[name];
}

Ref<RenderPassResource> RendererTools::CreateSharedRingBuffer(const String& name, UInt64 size, UInt64 stride)
{
    Ref<RenderPassResource> resource = MakeRef<RenderPassResource>();
    resource->Type = RenderPassResourceType::SharedRingBuffer;
    resource->ParentRHI = sData.RHI;
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        resource->RBuffer[i] = sData.RHI->CreateBuffer(size, stride, BufferType::Constant, name + " " + std::to_string(i));
        resource->RBuffer[i]->BuildCBV();
        if (stride > 0)
            resource->RBuffer[i]->BuildSRV();
        resource->RBuffer[i]->Tag(ResourceTag::RenderPassIO);
    }
    sData.Resources[name] = resource;
    return sData.Resources[name];
}

Ref<RenderPassResource> RendererTools::CreateSharedRWBuffer(const String& name, UInt64 size, UInt64 stride)
{
    Ref<RenderPassResource> resource = MakeRef<RenderPassResource>();
    resource->Type = RenderPassResourceType::SharedBuffer;
    resource->ParentRHI = sData.RHI;
    resource->Buffer = sData.RHI->CreateBuffer(size, stride, BufferType::Storage, name);
    resource->Buffer->BuildSRV();
    resource->Buffer->BuildUAV();
    resource->Buffer->Tag(ResourceTag::RenderPassIO);
    sData.Resources[name] = resource;
    return sData.Resources[name];
}

Ref<RenderPassResource> RendererTools::CreateSharedSampler(const String& name, SamplerFilter filter, SamplerAddress address, bool mips, bool comparison)
{
    Ref<RenderPassResource> resource = MakeRef<RenderPassResource>();
    resource->Type = RenderPassResourceType::SharedSampler;
    resource->ParentRHI = sData.RHI;
    resource->Sampler = sData.RHI->CreateSampler(address, filter, mips, 16, comparison);
    sData.Resources[name] = resource;
    return sData.Resources[name];
}

Ref<RenderPassResource> RendererTools::Get(const String& name)
{
    return sData.Resources[name];
}
