#include "win.h"
#include <fstream>
bool dumpTexture(ID3D11Device *device, ID3D11Texture2D *texture,
                 const string &filename) {
  const char *dir = "texture";
  DWORD attrib = GetFileAttributesA(dir);
  if (attrib == INVALID_FILE_ATTRIBUTES ||
      !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
    if (!CreateDirectoryA(dir, NULL)) {
      std::cout << "Failed to create directory: " << dir << std::endl;
      return false;
    } else {
      std::cout << "Directory created: " << dir << std::endl;
    }
  } else {
    // already exists
  }

  D3D11_TEXTURE2D_DESC desc = {};
  ComPtr<ID3D11DeviceContext> deviceContext;
  HRESULT hr;
  texture->GetDesc(&desc);
  desc.Usage = D3D11_USAGE_STAGING;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  desc.BindFlags = 0;
  ComPtr<ID3D11Texture2D> stagingTexture;
  hr = device->CreateTexture2D(&desc, nullptr, stagingTexture.GetAddressOf());
  IF_FAILED_THROW(hr);
  device->GetImmediateContext(deviceContext.ReleaseAndGetAddressOf());
  deviceContext->CopyResource(stagingTexture.Get(), texture);

  D3D11_MAPPED_SUBRESOURCE mappedResource = {};
  deviceContext->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0,
                     &mappedResource);
  string path = string(dir) + "/" + filename;
  std::ofstream file(path, std::ios::binary | std::ios::app);
  if (desc.Format == DXGI_FORMAT_NV12) {
    int Pitch = mappedResource.RowPitch;
    uint8_t *Y = (uint8_t *)mappedResource.pData;
    uint8_t *U =
        (uint8_t *)mappedResource.pData + desc.Height * mappedResource.RowPitch;
    uint8_t *V = (desc.Format == DXGI_FORMAT_P010) ? U + 2 : U + 1;
    for (int i = 0; i < desc.Height; i++) {
      file.write((const char *)(Y + i * Pitch), desc.Width);
    }
    int ChromaH = desc.Height / 2;
    int ChromaW = desc.Width;
    for (int i = 0; i < ChromaH; i++) {
      file.write((const char *)(U + i * Pitch), ChromaW);
    }
  }

  file.close();
  return true;
}