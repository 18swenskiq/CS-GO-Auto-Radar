using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MCDV_Processor
{
    class Program
    {
        public static string dbFolder = @"D:\MCDV-MODEL-DATABASE\CONVERTED";

        static void Main(string[] args)
        {
            //runDatabaseConversion(@"D:\MCDV-MODEL-DATABASE\SOURCE", @"D:\MCDV-MODEL-DATABASE\CONVERTED");

            //Console.ReadLine();
            //return;

            //MCDV.BSPtoTBSP(@"testmaterial\ar_baggage.bsp", "test.tbsp");
            List<string> test_models = getModelList("de_mirage.tbsp");

            foreach(string mdl in test_models)
            {
                copyDBFileToFolder(mdl, "de_mirage.resources");
            }

            Console.ReadLine();
        }

        //Check if model in database
        public static bool checkModelInDatabase(string modelName)
        {
            if (File.Exists(Path.ChangeExtension(dbFolder + "/" + modelName, "tmdl")))
                return true;
            return false;
        }

        //Copys model from database to target dir
        public static void copyDBFileToFolder(string modelName, string targetFolder)
        {
            if(checkModelInDatabase(modelName))
            {
                string targetDirRel = Path.GetDirectoryName(modelName);
                if (!Directory.Exists(targetFolder + "/" + targetDirRel))
                    Directory.CreateDirectory(targetFolder + "/" + targetDirRel);

                File.Copy(Path.ChangeExtension(dbFolder + "/" + modelName, "tmdl"), Path.ChangeExtension(targetFolder + "/" + modelName, "tmdl"));
            }
        }

        public static void runDatabaseConversion(string sourceFolder)
        {
            //Find all .mdl files
            string[] files = Directory.GetFiles(sourceFolder, "*.mdl", SearchOption.AllDirectories);

            ProgressViewer viewer = new ProgressViewer("Converted model 0 of " + files.Length);

            int c = 0;

            foreach (string file in files)
            {
                string dir = Path.GetDirectoryName(file);
                string modelName = Path.GetFileNameWithoutExtension(file);

                string relativeFileDir = (dir + "/" + modelName).Substring(sourceFolder.Length);

                //Search for vtx file
                string[] suffixes = new string[] { ".dx90.vtx" };

                string vtx_path = "";
                string vvd_path = "";
                foreach (string suffix in suffixes)
                {
                    if (File.Exists(dir + "/" + modelName + suffix))
                        vtx_path = dir + "/" + modelName + suffix;
                }

                if (File.Exists(dir + "/" + modelName + ".vvd"))
                    vvd_path = dir + "/" + modelName + ".vvd";

                if (vtx_path != "" && vvd_path != "")
                {
                    //Console.WriteLine("Converting {0} to .TMDL", modelName);

                    string targetFile = dbFolder + "/" + relativeFileDir + ".tmdl";

                    //Create file if it does not exist
                    if (!Directory.Exists(Path.GetDirectoryName(targetFile)))
                        Directory.CreateDirectory(Path.GetDirectoryName(targetFile));

                    //Check / skip if file exits already
                    if (File.Exists(targetFile))
                        continue;


                    int status = MCDV.MDLDatatoTMDL(vtx_path, vvd_path, targetFile);


                }
                else
                {
                    //Console.WriteLine("Error searching for model vtx/vvd: {0}", file);
                }

                c++;

                viewer.title = "Converted model " + c + " of " + files.Length;

                viewer.percent = (float)c / (float)files.Length;
                viewer.Draw();
            }

            viewer.End();
        }

        //Grabs the model list out of the .tbsp file
        public static List<string> getModelList(string tbsp_path)
        {
            List<string> modelList = new List<string>();

            if (File.Exists(tbsp_path))
            {
                using (BinaryReader reader = new BinaryReader(File.Open(tbsp_path, FileMode.Open)))
                {
                    //Read through
                    int magicNum = reader.ReadInt32();
                    int version = reader.ReadInt32();

                    int meshCount = reader.ReadInt32();
                    int meshLocation = reader.ReadInt32();

                    int modelCount = reader.ReadInt32();
                    int modelLocation = reader.ReadInt32();

                    int modelDictCount = reader.ReadInt32();
                    int modelDictLocation = reader.ReadInt32();

                    int entityCount = reader.ReadInt32();
                    int entityLocation = reader.ReadInt32();

                    //Jump to modelDict location
                    reader.BaseStream.Seek(modelDictLocation, SeekOrigin.Begin);

                    for(int i = 0; i < modelDictCount; i++)
                    {
                        string build = "";
                        byte c;
                        while ((c = reader.ReadByte()) != '\0')
                            build += (char)c;

                        modelList.Add(build);
                    }
                }
            }

            return modelList;
        }
    }
}
