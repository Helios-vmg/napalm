using System;

namespace cs_napalm
{
    public interface ILevelSource
    {
        float[] GetLevels();
        void RegisterCallback(Action<float[]> callback);
    }
}