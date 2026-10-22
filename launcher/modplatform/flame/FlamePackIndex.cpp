// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (c) 2026 Trial97 <alexandru.tripon97@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "FlamePackIndex.h"

#include "Json.h"
#include "modplatform/ModIndex.h"
#include "modplatform/flame/FlameAPI.h"

namespace Flame::Parse {
Result<> loadIndexedPack(ModPlatform::IndexedPack& pack, const QJsonObject& obj)
{
    TRY_INTO(pack.addonId, Json::requireInteger(obj, "id"))
    pack.provider = ModPlatform::ResourceProvider::FLAME;
    TRY_INTO(pack.name, Json::requireString(obj, "name"))
    TRY_INTO(pack.slug, Json::requireString(obj, "slug"))
    pack.websiteUrl = obj["links"].toObject()["websiteUrl"].toString("");
    pack.description = obj["summary"].toString("");

    QJsonObject logo = obj["logo"].toObject();
    pack.logoName = logo["title"].toString();
    pack.logoUrl = logo["thumbnailUrl"].toString();
    if (pack.logoUrl.isEmpty()) {
        pack.logoUrl = logo["url"].toString();
    }

    auto authors = obj["authors"].toArray();
    if (!authors.isEmpty()) {
        pack.authors.clear();
        for (auto authorIter : authors) {
            TRY_INTO(const auto& author, Json::requireObject(authorIter))
            ModPlatform::ModpackAuthor packAuthor;
            TRY_INTO(packAuthor.name, Json::requireString(author, "name"))
            TRY_INTO(packAuthor.url, Json::requireString(author, "url"))
            pack.authors.append(packAuthor);
        }
    }

    pack.resourceType = FlameAPI::getResourceType(obj["classId"].toInt(0));
    pack.extraDataLoaded = false;

    if (obj.contains("downloadCount")) {
        pack.extraData.downloads = static_cast<quint64>(obj["downloadCount"].toVariant().toULongLong());
    }
    if (obj.contains("thumbsUpCount")) {
        pack.extraData.followers = static_cast<quint64>(obj["thumbsUpCount"].toVariant().toULongLong());
    }

    if (obj.contains("screenshots")) {
        pack.extraData.gallery.clear();
        for (const auto& item : obj["screenshots"].toArray()) {
            if (!item.isObject())
                continue;
            auto sObj = item.toObject();
            ModPlatform::GalleryItem g;
            g.url = sObj["thumbnailUrl"].toString();
            g.rawUrl = sObj["url"].toString();
            if (g.url.isEmpty()) {
                g.url = g.rawUrl;
            }
            g.title = sObj["title"].toString();
            g.description = sObj["description"].toString();
            pack.extraData.gallery.append(g);
        }
    }

    auto linksObj = obj["links"].toObject();

    pack.extraData.issuesUrl = linksObj["issuesUrl"].toString();
    if (pack.extraData.issuesUrl.endsWith('/')) {
        pack.extraData.issuesUrl.chop(1);
    }

    pack.extraData.sourceUrl = linksObj["sourceUrl"].toString();
    if (pack.extraData.sourceUrl.endsWith('/')) {
        pack.extraData.sourceUrl.chop(1);
    }

    pack.extraData.wikiUrl = linksObj["wikiUrl"].toString();
    if (pack.extraData.wikiUrl.endsWith('/')) {
        pack.extraData.wikiUrl.chop(1);
    }

    if (!pack.extraData.body.isEmpty()) {
        pack.extraDataLoaded = true;
    }
    return {};
}
}  // namespace Flame::Parse
