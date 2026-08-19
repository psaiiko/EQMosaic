using System;

using Vortice.D3DCompiler;
using Vortice.Direct3D;
using Vortice.Direct3D11;

public class ShaderCompiler
{
    public ShaderCompiler(ID3D11Device device)
    {
        Device = device;
    }

    public enum ShaderType
    {
        PixelShader,
        VertexShader,
    }

    public ID3D11Device Device { get; private set; }

    public byte[] CompileFromFile(ShaderType shaderType, string fileName, string entryPoint = null, ShaderFlags shaderFlags = ShaderFlags.None)
    {
#if DEBUG
        shaderFlags |= ShaderFlags.Debug;
#endif
        var profile = GetProfile(shaderType);
        var result = Compiler.CompileFromFile(fileName, entryPoint, profile, shaderFlags);
        return result.ToArray();
    }

    private string GetProfile(ShaderType shaderType)
    {
        return shaderType switch
        {
            ShaderType.VertexShader => "vs_5_0",
            ShaderType.PixelShader => "ps_5_0",
            _ => throw new ArgumentException("Unknown shader type"),
        };
    }
}
