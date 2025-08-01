#include "ConfigHelper.hpp"

std::string ConfigHelper::SaveConfig::save_2d_image_file(const std::string& suffix) const
{
    return  this->save_path + "/" + suffix + "_2DImage.png";
}

std::string ConfigHelper::SaveConfig::save_depth_map_file(const std::string& suffix) const
{
    return this->save_path + "/" + suffix + "_DepthMap.tiff";
}

std::string ConfigHelper::SaveConfig::save_point_cloud_file(const std::string& suffix) const
{
    return this->save_path + "/" + suffix + "_PointCloud.ply";
}

std::string ConfigHelper::SaveConfig::save_textured_point_cloud_file(const std::string& suffix) const
{
    return this->save_path + "/" + suffix + "_TexturedPointCloud.ply";
}   