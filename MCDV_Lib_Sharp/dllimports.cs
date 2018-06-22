using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace MCDV_Lib_Sharp
{
    public class MCDV
    {
        [DllImport("MCDV_Lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int BSPtoTBSP(string path, string output);

        [DllImport("MCDV_Lib.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int MDLDatatoTMDL(string path_vtex, string path_vvd, string output);
    }
}
