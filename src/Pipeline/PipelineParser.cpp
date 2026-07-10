#include "PipelineParser.h"

#include <tinyxml2.h>

#include <stdexcept>
#include <iostream>

namespace
{
std::string attributeOrEmpty(const tinyxml2::XMLElement& element, const char* name)
{
    const char* value = element.Attribute(name);
    return value != nullptr ? value : "";
}

PipelineCommand parseSwitchTarget(const tinyxml2::XMLElement& element)
{
    PipelineCommand command;
    command.type = PipelineCommandType::SwitchTarget;
    command.target = attributeOrEmpty(element, "target");
    return command;
}

PipelineCommand parseBindBuffer(const tinyxml2::XMLElement& element)
{
    PipelineCommand command;
    command.type = PipelineCommandType::BindBuffer;
    command.sampler = attributeOrEmpty(element, "sampler");
    command.sourceRT = attributeOrEmpty(element, "sourceRT");
    element.QueryIntAttribute("bufIndex", &command.bufIndex);
    return command;
}

PipelineCommand parseClearTarget(const tinyxml2::XMLElement& element)
{
    PipelineCommand command;
    command.type = PipelineCommandType::ClearTarget;
    element.QueryBoolAttribute("depthBuf", &command.clearDepth);
    element.QueryBoolAttribute("colBuf0", &command.clearColor0);
    element.QueryFloatAttribute("col_R", &command.clearR);
    element.QueryFloatAttribute("col_G", &command.clearG);
    element.QueryFloatAttribute("col_B", &command.clearB);
    element.QueryFloatAttribute("col_A", &command.clearA);
    element.QueryFloatAttribute("depth", &command.clearDepthValue);
    return command;
}

PipelineCommand parseDrawGeometry(const tinyxml2::XMLElement& element)
{
    PipelineCommand command;
    command.type = PipelineCommandType::DrawGeometry;
    command.context = attributeOrEmpty(element, "context");
    command.cullFrustum = attributeOrEmpty(element, "cullFrustum");
    return command;
}

PipelineCommand parseDrawQuad(const tinyxml2::XMLElement& element)
{
    PipelineCommand command;
    command.type = PipelineCommandType::DrawQuad;
    command.material = attributeOrEmpty(element, "material");
    command.context = attributeOrEmpty(element, "context");
    return command;
}

PipelineCommand parseUnbindBuffers()
{
    PipelineCommand command;
    command.type = PipelineCommandType::UnbindBuffers;
    return command;
}

const char* commandTypeName(PipelineCommandType type)
{
    switch (type)
    {
    case PipelineCommandType::SwitchTarget:
        return "SwitchTarget";
    case PipelineCommandType::ClearTarget:
        return "ClearTarget";
    case PipelineCommandType::BindBuffer:
        return "BindBuffer";
    case PipelineCommandType::DrawGeometry:
        return "DrawGeometry";
    case PipelineCommandType::DrawQuad:
        return "DrawQuad";
    case PipelineCommandType::UnbindBuffers:
        return "UnbindBuffers";
    }

    return "Unknown";
}

void printPipelineCommands(const std::vector<PipelineStage>& stages)
{
    std::cout << "Parsed pipeline stages:\n";

    for (const PipelineStage& stage : stages)
    {
        std::cout << "  Stage id=\"" << stage.id << "\" enabled=" << (stage.enabled ? "true" : "false") << '\n';

        for (const PipelineCommand& command : stage.commands)
        {
            std::cout << "    " << commandTypeName(command.type);

            if (!command.target.empty())
            {
                std::cout << " target=\"" << command.target << "\"";
            }

            if (!command.sampler.empty())
            {
                std::cout << " sampler=\"" << command.sampler << "\"";
            }

            if (!command.sourceRT.empty())
            {
                std::cout << " sourceRT=\"" << command.sourceRT << "\"";
            }

            if (command.type == PipelineCommandType::BindBuffer)
            {
                std::cout << " bufIndex=" << command.bufIndex;
            }

            if (command.type == PipelineCommandType::ClearTarget)
            {
                std::cout << " color=" << (command.clearColor0 ? "true" : "false")
                          << " depth=" << (command.clearDepth ? "true" : "false");
            }

            if (!command.material.empty())
            {
                std::cout << " material=\"" << command.material << "\"";
            }

            if (!command.context.empty())
            {
                std::cout << " context=\"" << command.context << "\"";
            }

            if (!command.cullFrustum.empty())
            {
                std::cout << " cullFrustum=\"" << command.cullFrustum << "\"";
            }

            std::cout << '\n';
        }
    }
}

}

std::vector<PipelineStage> PipelineParser::load(const std::string& path) const
{
    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError loadResult = document.LoadFile(path.c_str());
    if (loadResult != tinyxml2::XML_SUCCESS)
    {
        throw std::runtime_error("Failed to load pipeline XML: " + path);
    }

    const tinyxml2::XMLElement* pipeline = document.FirstChildElement("Pipeline");
    if (pipeline == nullptr)
    {
        throw std::runtime_error("Pipeline XML is missing <Pipeline>: " + path);
    }

    const tinyxml2::XMLElement* commandQueue = pipeline->FirstChildElement("CommandQueue");
    if (commandQueue == nullptr)
    {
        return {};
    }

    std::vector<PipelineStage> stages;
    for (const tinyxml2::XMLElement* stageElement = commandQueue->FirstChildElement("Stage");
         stageElement != nullptr;
         stageElement = stageElement->NextSiblingElement("Stage"))
    {
        PipelineStage stage;
        stage.id = attributeOrEmpty(*stageElement, "id");
        stageElement->QueryBoolAttribute("enabled", &stage.enabled);

        for (const tinyxml2::XMLElement* commandElement = stageElement->FirstChildElement();
             commandElement != nullptr;
             commandElement = commandElement->NextSiblingElement())
        {
            const std::string commandName = commandElement->Name();

            if (commandName == "SwitchTarget")
            {
                stage.commands.push_back(parseSwitchTarget(*commandElement));
            }
            else if (commandName == "ClearTarget")
            {
                stage.commands.push_back(parseClearTarget(*commandElement));
            }
            else if (commandName == "BindBuffer")
            {
                stage.commands.push_back(parseBindBuffer(*commandElement));
            }
            else if (commandName == "DrawGeometry")
            {
                stage.commands.push_back(parseDrawGeometry(*commandElement));
            }
            else if (commandName == "DrawQuad")
            {
                stage.commands.push_back(parseDrawQuad(*commandElement));
            }
            else if (commandName == "UnbindBuffers")
            {
                stage.commands.push_back(parseUnbindBuffers());
            }
        }

        stages.push_back(stage);
    }

    printPipelineCommands(stages);

    return stages;
}
