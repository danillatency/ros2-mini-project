import pathlib

rviz_sdf = open(pathlib.Path(__file__).parent.parent / "robots" / "deliverer_template.sdf", 'r', encoding="utf-8").read(
).replace("{{ DELIVERER_NAME }}", "deliverer_0"
          ).replace("<link name=\"", "<link name=\"deliverer_0/"
                    ).replace("<joint name=\"", "<joint name=\"deliverer_0/"
                              ).replace("<parent>", "<parent>deliverer_0/"
                                        ).replace("<child>", "<child>deliverer_0/")
open(pathlib.Path(__file__).parent.parent / "rviz" / "deliverer_0.sdf", 'w', encoding="utf-8").write(rviz_sdf)
