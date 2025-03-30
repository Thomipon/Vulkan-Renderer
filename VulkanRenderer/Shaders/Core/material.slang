module material;

import brdf;
import geometry;

public struct MaterialResult<BRDF : IBRDF>
{
    public BRDF brdf;
    public SurfaceGeometry geometry;
}

public interface IMaterial
{
    associatedtype BRDF : IBRDF;

    MaterialResult<BRDF> evaluate(SurfaceGeometry geometry);
}