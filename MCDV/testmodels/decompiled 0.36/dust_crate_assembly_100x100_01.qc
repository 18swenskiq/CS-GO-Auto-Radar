// Created by Crowbar 0.36.0.0

$ModelName "props\de_dust\hr_dust\dust_crates\dust_crate_assembly_100x100_"

$StaticProp

$BodyGroup "Body"
{
	studio "dust_crate_assembly_100x100_01_reference.smd"
}


$SurfaceProp "Wood_Plank"

$Contents "solid"

$MaxEyeDeflection 90

$CDMaterials "models\props\de_dust\hr_dust\dust_crates\"
$CDMaterials ""

$TextureGroup "skinfamilies"
{
	{ "models\props\de_dust\hr_dust\dust_crates\dust_shipping_crate_01_wood_color" "models\props\de_dust\hr_dust\dust_crates\dust_shipping_crate_02_wood_color" }
	{ "dust_shipping_crate_01_wood_decals"                                         "dust_shipping_crate_02_wood_decals"                                         }
	{ "dust_shipping_crate_01_wood_decals"                                         "dust_shipping_crate_02_wood_decals"                                         }
	{ "dust_shipping_crate_01_painted_color"                                       "dust_shipping_crate_02_painted_color"                                       }
	{ "dust_shipping_crate_01_painted_decals"                                      "dust_shipping_crate_02_painted_decals"                                      }
}

$CBox 0 0 0 0 0 0

$BBox -51.056 -51.429 -0.263 50.999 54.908 74.733


$Sequence "idle" {
	"dust_crate_assembly_100x100_01_anims\idle.smd"
	activity "act_idle" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$CollisionModel "dust_crate_assembly_100x100_01_physics.smd"
{
	$mass 7131.675
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave

}

$KeyValues
{
	qc_path 
	{
		"value" "models\props\de_dust\hr_dust\dust_crates\dust_crate_assembly_100x100_01.qc"
	}
}
