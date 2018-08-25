using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace cs_napalm
{
    public class FullTrackMetadata
    {
        public readonly BasicTrackInfo BasicTrackInfo;
        public readonly byte[] FrontCover;
        public Dictionary<string, string> Metadata;
    }
}
